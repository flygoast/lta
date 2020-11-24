#ifndef __LTE_APPLICATION_H__
#define __LTE_APPLICATION_H__


#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Mutex.h"

using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;

using Poco::Mutex;


#define LOG_PATH    "log"
#define DATA_PATH   "data"


namespace LTE {

class Application: public ServerApplication {
public:
    Application(void);

    void initialize(Poco::Util::Application& self);
    void uninitialize(void);
    void reinitialize(Poco::Util::Application& self);
    void defineOptions(OptionSet& options);

    void configureProxy(Poco::Util::Application& self);
    void configureLogging(Poco::Util::Application& self);
    void handleOption(const std::string& name, const std::string& value);
    void displayHelp();
    void displayVersion();
    void exit(int code);

    int main(const std::vector<std::string>& args);

    int checkFDs(int maxfd);

    std::string getDaemonKey();

    Mutex           mutex;
    std::string     log_file;
    std::string     store_file;
    std::string     id_file;

    bool            controlled;
    bool            version;
    bool            help;
};

}; // namespace LTE

#endif /* __LTE_APPLICATION_H__ */
