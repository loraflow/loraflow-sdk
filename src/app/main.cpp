#include <app/App.h>
#include <core/api.hpp>
#include <hythread.h>


using namespace haul;
using namespace hythread;
using etc = conf::etc;

Log::Logger Log::_logger = spdlog::stdout_color_mt("console");
conf::Config conf::INSTANCE;

App app;
WebAPI web{};
int hylength;
bool isReset = false;

namespace hythread {

    void startCountThread() {
        web.setUpgradeStart();
    }

    void stopCountThread() {
        INFOF("接收LENGTH...{}",hylength);
    }
}




extern "C" void _on_signal(int sig) {
    WARNF("caught signal {}", sig);
    exit(-1);
}



void threadFuncMain(std::string &str, int argc, char **argv)
{
    os::enable_int(_on_signal);

    os::handle_pipe();

    Log::_logger->set_pattern("[%Y%m%d %T.%e] %v");

    std::srand(std::time(0));

    auto &cf = conf::INSTANCE;

    cf.init(argc, argv);
    cf.writeVersion(os::version(),os::hwversion(),os::gwmodel());

    PRINTF("starting nanohaul version FW:{},HW:{},MDL:{}...", os::version().data(),os::hwversion().data(),os::gwmodel().data());

    if (cf.c_local().system.loglevel != "") {
        Log::set_level(cf.c_local().system.loglevel);
    }

    app.run();
}

void threadFuncApi(std::string &str, int a)
{
    PRINTF("Web Server Start{}...",os::version().data());
    Json conf = conf::INSTANCE.Get();
    std::map<string, Cnode*> nodesmap{};
    try {
        string host = "0.0.0.0";
        int port = 8000;
        if (!host.empty() && port > 0) {
            web.Bind(nodesmap, host, port);
        } else {
            INFOF("hmi disabled");
        }
    } catch (...) {
        WARNF("hmi disabled");
    }
    while (web.Running()) {
        lang::os::sleep_ms(50);
    }
    INFOF("Bye World!");
}

void threadFuncReset(std::string &str, int a)
{
    INFOF("Web Server Reset Thread Start...");

    while (1) {
        if(isReset) {
            isReset = false;
            lang::os::sleep_ms(5000);
            lang::os::reboot();
        }
        lang::os::sleep_ms(50);
    }
//    INFOF("Bye Reset Thread!");
}


int main(int argc, char **argv) {

#if 1
    std::string str("threadFuncMain");
    std::string str1("threadFuncApi");
    std::string str2("threadFuncReset");
    std::thread th2(threadFuncApi, std::ref(str1),9);
    std::thread th3(threadFuncMain, std::ref(str),argc,argv);
    std::thread th4(threadFuncReset, std::ref(str2),10);
    th2.join();
    th3.join();
    th4.join();
#else
    Json conf = conf::INSTANCE.Get();
    std::map<string, Cnode*> nodesmap{};
    try {
        string host = "0.0.0.0";
        int port = 8000;
        if (!host.empty() && port > 0) {
            web.Bind(nodesmap, host, port);
        } else {
            INFOF("hmi disabled");
        }
    } catch (...) {
        WARNF("hmi disabled");
    }

    std::ofstream dst("./www/sendtest/text.zip");
    while (web.Running()) {
        lang::os::sleep_ms(50);
    }
    INFOF("Bye World!");

#endif


    return 0;
}
