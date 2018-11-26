#include <app/App.h>

using namespace haul;
using etc = conf::etc;

Log::Logger Log::_logger = spdlog::stdout_color_mt("console");
conf::Config conf::INSTANCE;

App app;

extern "C" void _on_signal(int sig) {
    WARNF("caught signal {}", sig);
    exit(-1);
}

int main(int argc, char **argv) {

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

    return 0;
}
