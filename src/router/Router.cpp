#include <lang/errors.h>
#include <lang/lang-json.h>
#include <router/Router.h>
#include <types/jsonutils.h>
#include <conf/config.h>
#include <lang/lang-rest.h>

using namespace lang;
using namespace lang::errors;
using namespace haul;

static string cmd_exec(Json &opt, Json &out);
static string cmd_shell(Json &opt, Json &out);
static string cmd_readtxt(Json &opt, Json &out);
static string cmd_writetxt(Json &opt, Json &out);
static string cmd_fcopy(Json &opt, Json &out);
static string cmd_ping(Json &opt, Json &out);
static string cmd_confset(Json &opt, Json &out);
static string cmd_confget(Json &opt, Json &out);
static string cmd_upgrade(Json &opt, Json &out);
extern string version();
typedef string (*CommandHandler)(Json &opt, Json &out);

using Commands = map<string, CommandHandler>;
Commands _commands = {
        {"ping", cmd_ping},
        {"exec", cmd_exec},
        {"shell", cmd_shell},
        {"readtxt", cmd_readtxt},
        {"txtwr", cmd_writetxt},
        {"fread", cmd_readtxt},
        {"fwrite", cmd_writetxt},
        {"fcopy", cmd_fcopy},
        {"cs", cmd_confset},
        {"cg", cmd_confget},
        {"upg", cmd_upgrade},
        {"kread", cmd_confset},
        {"kwrite", cmd_confget},
};

typedef struct {
    string command;
    string *results;
} Async;

static string _err2res(ErrStatus err, string more = "") {
    string str = err == ERR_OK ? "+OK" : "err:"+to_string(err);
    if (more.size()) {
        str += "\n" + more;
    }
    return str;
}

class ShellCmd {
    string _command;
    string _result;
public:
    ShellCmd(string c):_command(c) {}

    string run() {
        std::thread _th = std::thread(&ShellCmd::daemon, this);
        _th.join();
        return _result;
    }

protected:
    void daemon() {
        FILE *fp = popen(_command.c_str(), "r");
        if (fp == NULL) {
            _result = "err: failed to run command";
        } else {
            char staticbuff[4096];  //warn: unsafe!
            staticbuff[0] = '\0';
            char *ptr = staticbuff;
            while (fgets(ptr, sizeof(staticbuff) - (ptr - staticbuff), fp) != NULL) {
                const int N = strlen(ptr);
                if (N == 0) {
                    char buf[10];
                    while (fgets(buf, sizeof(buf), fp) != NULL) {
                    }
                    break;
                }
                ptr += N;
            }
            pclose(fp);
            _result = string(staticbuff);
        }
        DEBUGF("Shell cmd {} Result {}", _command, _result);
    }
};

void Router::_handle_aisenz_down(AisenzDownMessage *message) {
    do {
        Json jup;
        bool valid = false;
        try {
            Json &jdown = message->content;
            string act = jdown["act"]; //json_object_get_string(root, "act");
            Json opt = jdown["opt"];

            if (act.size() == 0) {
                break;
            }
            const Commands::iterator c = _commands.find(act);
            Json reopts;
            if (c == _commands.end()) {
                reopts["res"] = "+UNKNOWN";
            } else {
                try {
                    string status = c->second(opt, reopts);
                    if (status.size()) {
                        reopts["res"] = status;
                    }
                } catch (...) {
                    std::exception_ptr p = std::current_exception();
                    DEBUGF("req handling error ({})", p ? p.__cxa_exception_type()->name() : "null");
                    reopts["res"] = "+err";
                }
            }
            if (!reopts.empty()) {
                jup["opt"] = reopts;
            }
            jup["act"] = act;
            jup["ack"] = jdown["id"];
            valid = true;
        } catch(...) {
            std::exception_ptr p = std::current_exception();
            DEBUGF("err in request ({})", p ? p.__cxa_exception_type()->name() : "null");
        }
        if (valid) {
            jup["E"] = _starttime;
            jup["U"] = os::epoch() - _starttime;  //uptime
            AisenzUpMessage *up = new AisenzUpMessage;
            up->content = jup;
            unique_ptr<Message> p(up);
            _up.put(std::move(p));
        }
    } while (0);
}

static inline string _part_file(int part) {
    return ".lorabridge-part-" + part;
}

static inline string archivename(string file) {
    return file + ".LORABRIDGE.BAK";
}

static inline string bakname(string file) {
    return file + ".tmp.bak";
}

class Upgrader {
    std::thread _th;
    bool _running = false;
    string _url;
    string _saveas;
public:
    void start(const string &url, const string &saveas) {
        if (!_running) {
            _running = true;
            _url = url;
            _saveas = saveas;
            _th = std::thread(&Upgrader::_daemon, this);
        }
    }
    bool isRunning() const {
        return _running;
    }

protected:
    void _daemon() {
        INFOF("start upgrading {}", _saveas);
        if (!lang::download(_url, _saveas)) {
            ERRORF("upgrade failed!");
        } else {
            WARNF("ABOUT TO UPGRADE!");
            ::exit(0);
        }
        _running = false;
    }
};

static string cmd_upgrade(Json &opt, Json &outopt) {
    static Upgrader _upgrader;
    const string url = opt["url"];
    const string saveas = opt["as"];
    if (url.empty() || saveas.empty()) {
        return "err:badarg";
    }
    if (_upgrader.isRunning()) {
        return "err:busy";
    }
    _upgrader.start(url, saveas);
    return "+OK";
}

static string cmd_fcopy(Json &opt, Json &outopt) {
    string src = opt["src"];
    string dst = opt["dst"];
    if (src.size() == 0 || dst.size() == 0) {
        return "err:badarg";
    }
    if (src == "$ARCHIVE") {
        src = archivename(dst);
    } else if (src == "$BACKUP") {
        src = bakname(dst);
    }
    if (!os::fexists(src)) {
        DEBUGF("file {} not exists", src);
        return "err:notexists";
    }
    return os::fcopy(dst, src, true) ? "+OK" : "err:io";
}

static string cmd_writetxt(Json &opt, Json &outopt) {
#define TIMEOUT 60000

    static int _lastpart = 0;
    static uint64_t _lastmills = 0;
    static size_t _nbytes = 0;
    string result;
    do {
        string data = opt["data"];
        string path, bak;
        jsons::optional(opt, "file", path);
        jsons::optional(opt, "bak", bak);
        const int part = opt["part"];
        const bool final = path.size() > 0;

        if (part < 0 || part>1000 || data.size() == 0) {
            result = "err:badarg";
            break;
        }
        if (part > 0) {
            if (_lastmills == 0 || _lastpart +1 > part) {  //dup, ignored
                break;
            }
            if (_lastpart +1 < part) {
                result = "err:broken";
                break;
            }
            if (os::mills() - _lastmills > TIMEOUT) {
                result = "err:expired";
                break;
            }
        } else {
            _nbytes = 0;
        }
        if (path.size()) {
            os::fcopy(archivename(path), path, false);
        }
        FILE *fp = fopen((part == 0 && final ? path : _part_file(part)).c_str(), "w");
        if (!fp) {
            result = "err:io";
            break;
        }

        const size_t dlen = data.size();
        if (dlen != fwrite(data.c_str(), 1, dlen, fp)) {
            result = "err:io";
            fclose(fp);
            break;
        }

        fclose(fp);

        _nbytes += dlen;
        _lastpart = part;
        _lastmills = !final ? os::mills() : 0;

        do {
            if (!final || !part) {
                break;
            }
            //now we are about to join part files

            if (bak.size() == 0) {
                bak = bakname(path);
            }
            os::fcopy(bak, path, true);
            FILE *foutput = fopen(path.c_str(), "w");
            if (!foutput) {
                result = "err:io";
                break;
            }
            size_t nbytes = 0;
            int itor1, itor2;
            for (itor1=0; itor1<=part; itor1++) {
                FILE *fpart = fopen(_part_file(itor1).c_str(), "r");
                if (!fpart) {
                    result = "err:io";
                    ERRORF("open part {}", itor1);
                    break;
                }
                char buf[1024];
                for (itor2=0; itor2<10000; itor2++) {
                    size_t nr = fread(buf, 1, sizeof(buf), fpart);
                    if (nr > 0) {
                        size_t nw = fwrite(buf, 1, nr, foutput);
                        if (nw != nr) {
                            result = "err:io";
                            ERRORF("write output");
                            break;
                        }
                        nbytes += nr;
                    }
                    if (nr < sizeof(buf)) {
                        break;
                    }
                }
                fclose(fpart);
                remove(_part_file(itor1).c_str());
            }
            fclose(foutput);
            if (nbytes != _nbytes) {
                result = "err:io";
                ERRORF("write {}, read {}", _nbytes, nbytes);
            }
            if (result.size()) {
                bool ok = os::fcopy(path, bak, true);
                DEBUGF("Faild write {}. Recover {}", path, ok ? "Succeed" : "Failed");
            } else {
                DEBUGF("OK write {}", path);
            }
        } while(0);
    } while (0);
    return result.size() ? result : "+OK";
}

static string cmd_readtxt(Json &opt, Json &outopt) {
#define MAXSIZE 2048
    static char sbuff[MAXSIZE + 100];
    char buff[MAXSIZE+1];
    string result;
    do {
        string path = opt["file"];
        long offset = jsons::getdefault(opt, "offset", 0l);
        long limit = jsons::getdefault(opt, "limit", (long)MAXSIZE);

        if (!path.size()) {
            result = "err:missingfile";
            break;
        }
        if (limit < 0 || offset < 0) {
            result = "err:arg";
            break;
        }
        if (limit > MAXSIZE) {
            result = "err:overflow";
            break;
        }
        FILE *fr = fopen (path.c_str(), "r");
        if (!fr) {
            result = "err:io";
            break;
        }
        if (0 != fseek(fr, offset, SEEK_SET)) {
            result = "err:io";
            fclose(fr);
            break;
        }
        int n = fread(buff, 1, limit+1, fr);
        if (n < 0) {
            result = "err:io";
        } else {
            int more = n > limit;
            if (more) {
                n = limit;
            }
            buff[n] = '\0';
            sprintf(sbuff, "{\"offset:\":%ld,\"mark\":%ld,\"more\":%s}\n%s",
                    offset, offset + n, more ? "true":"false", buff);
            result = sbuff;
        }
        fclose(fr);
    } while (0);
    return result;
}


static string cmd_shell(Json &opt, Json &outopt) {
    string line;
    if (!jsons::optional(opt, "cmd", line)) {
        return _err2res(errors::ERR_BADREQ);
    }
    ShellCmd cmd(line);
    return cmd.run();
}

static string cmd_confset(Json &opt, Json &outopt) {
    auto &c = conf::INSTANCE;
    string key = opt["k"];
    string val = opt["v"];
    auto err = errors::ERR_NOT_FOUND;
    if (key == "token") {
        c.local().networkservice.external.token = val;
        err = c.save_local();
    }
    return _err2res(err);
}

static string cmd_confget(Json &opt, Json &outopt) {
    auto &c = conf::INSTANCE;
    string key = opt["k"];
    if (key == "token") {
        return "+OK\n" + c.c_local().networkservice.external.token;
    }
    return _err2res(errors::ERR_NOT_FOUND);
}

static string cmd_ping(Json &opt, Json &outopt) {
    outopt["t0"] = opt["t0"];
    //int cage = epoch() - global.ctime;
    //if (cage <= 10) {
    //    json_object_set_integer(outopt, "cage", cage);
    //}
    return _err2res(errors::ERR_OK);
}

static string cmd_exec(Json &opt, Json &outopt) {
    string cmd = opt["cmd"];
    Json args = opt["args"];
    for (Json::iterator it = args.begin(); it != args.end(); ++it) {
        Json a = *it;
        cmd += " " + a.get<string>();
    }
    if (cmd == "reboot") {
        lang::os::reboot();
    } else if (cmd == "resetpf") {
        conf::INSTANCE.applyChanges(true);
    } else {
        int rc = system(cmd.c_str());
        DEBUGF("exec {} = {}", cmd, rc);
        switch (rc) {
            default:
            case -1:
            case 127:
                // The system() function returns the exit status of the shell as returned by waitpid(2),
                // or -1 if an error occurred when invoking fork(2) or waitpid(2).  A
                // return value of 127 means the execution of the shell failed.
                return "err:exitcode=" + rc;
            case 0:
                break;
        }
    }
    return _err2res(errors::ERR_OK);
}
