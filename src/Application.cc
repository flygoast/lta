#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "Poco/TaskManager.h"
#include "Poco/Task.h"
#include "Poco/File.h"
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Message.h"
#include "Poco/NumberParser.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/RegExpValidator.h"


#include "Application.h"
#include "MainLoop.h"
#include "Utils.h"
#include "Store.h"
#include "Version.h"


using Poco::TaskManager;
using Poco::File;
using Poco::FileChannel;
using Poco::Logger;
using Poco::FormattingChannel;
using Poco::PatternFormatter;
using Poco::Message;
using Poco::Util::HelpFormatter;
using Poco::Util::RegExpValidator;


LTE::Application::Application(void) {
    std::string file("lte.log");
    std::string dir("log");

    log_file = Utils::daemonFile(dir, file);

    file = "store.db";
    dir = "data";
    store_file = Utils::daemonFile(dir, file);

    file = "id";
    dir = "data";
    id_file = Utils::daemonFile(dir, file);

    struct stat statbuf;
    if (fstat(4, &statbuf) == -1 && fstat(5, &statbuf) == 0) {
        controlled = true;
    } else {
        controlled = false;
    }
}


void LTE::Application::initialize(Poco::Util::Application& self) {
    ServerApplication::initialize(self);

    std::string configFile("lte.properties");
    std::string configDir("data");
    std::string filePath = Utils::daemonFile(configDir, configFile);

    Poco::File file(filePath);

    if (file.exists() && file.isFile()) {
        loadConfiguration(filePath);
        file.remove();
    }

    close(3);

    configureLogging(self);
    configureProxy(self);
}


void LTE::Application::configureProxy(Poco::Util::Application& self) {
    Poco::Logger& logger = Poco::Util::Application::instance().logger();

    if (self.config().has("proxy-password")) {
        if (!self.config().has("proxy-user")) {
            std::cout << "'proxy-password' option requires 'proxy-user' option" << std::endl;
            logger.fatal("'proxy-password' option requires 'proxy-user' option");
            Poco::Util::ServerApplication::terminate();
        }
    }

    if (self.config().has("proxy-user")) {
        if (!self.config().has("proxy")) {
            std::cout << "'proxy-user' option requires 'proxy' option" << std::endl;
            logger.fatal("'proxy-user' option requires 'proxy' option");
            Poco::Util::ServerApplication::terminate();
        }
    }

    Store::AgentInfo ai;
    Store::getAgentInfo(ai);

    if (self.config().has("noproxy")) {
        ai.proxy_host.erase();
        ai.proxy_port = 0;
        ai.proxy_user.erase();
        ai.proxy_password.erase();
    } else if (self.config().has("proxy")) {
        std::string proxy = self.config().getString("proxy");
        int pos = proxy.rfind(':', -1);
        if (pos == -1) {
            std::cout << "'proxy' option requires a port be specificied" << std::endl;
            logger.fatal("'proxy' option requires a port be specificied");
            Poco::Util::ServerApplication::terminate();
        }

        ai.proxy_host = proxy.substr(0, pos);

        int len = ai.proxy_host.length();
        if (len == 0) {
            std::cout << "'proxy' option requires a host be specificied" << std::endl;
            logger.fatal("'proxy' option requires a host be specificied");
            Poco::Util::ServerApplication::terminate();
        }

        if (ai.proxy_host.at(0) == '[' && ai.proxy_host.at(len - 1) == ']') {
            ai.proxy_host = ai.proxy_host.substr(1, len - 2);
        }

        if (pos + 1 <= proxy.length()) {
            if (!Poco::NumberParser::tryParseUnsigned(proxy.substr(pos + 1, -1), ai.proxy_port)) {
                std::cout << "proxy port is not an unsigned integer" << std::endl;
                logger.fatal("proxy port is not an unsigned integer");
                Poco::Util::ServerApplication::terminate();
            }

            if (ai.proxy_port >= 65535) {
                std::cout << "proxy port is not an valid integer" << std::endl;
                logger.fatal("proxy port is not an valid integer");
                Poco::Util::ServerApplication::terminate();
            }
        }

        if (self.config().has("proxy-user")) {
            ai.proxy_host = self.config().getString("proxy-user");
            ai.proxy_password = self.config().getString("proxy-password");
        }
    }

    Store::setAgentInfo(ai); 
}


void LTE::Application::configureLogging(Poco::Util::Application& self) {
    std::string name;

    Store::AgentInfo ai;
    Store::getAgentInfo(ai);

    if (self.config().has("enable-log-rotation")) {
        ai.enable_log_rotation = true;
        ai.max_log_size = self.config().getInt("max-log-size");
        ai.max_log_files = self.config().getInt("max-log-files");

    } else {
        if (self.config().has("disable-log-rotation")) {
            ai.enable_log_rotation = false;
        }
    }
    Store::setAgentInfo(ai); 
    Store::closeSession();

    name = "debug";
    bool debug = self.config().getBool(name, false);

    FormattingChannel* pFCFile = new FormattingChannel(new PatternFormatter("%Y-%m-%d %H:%M:%S.%c %N[%P] %p - %t"));
    FileChannel* fc = new FileChannel(log_file);

    if (ai.enable_log_rotation) {
        std::string s = std::to_string(ai.max_log_size);
        s.append(" M");
        fc->setProperty("rotation", s);
        fc->setProperty("archive", "number");
        fc->setProperty("compress", "true");
        s = std::to_string(ai.max_log_files);
        fc->setProperty("purgeCount", s);
    }

    pFCFile->setChannel(fc);
    pFCFile->open();

    Logger& fileLogger = Logger::create("FileLogger", pFCFile, (debug? Message::PRIO_DEBUG: Message::PRIO_INFORMATION));
    setLogger(fileLogger);
}


void LTE::Application::uninitialize() {
    Poco::Util::Application::uninitialize();
}

void LTE::Application::reinitialize(Poco::Util::Application& self) {
    Poco::Util::ServerApplication::reinitialize(self);
}

void LTE::Application::displayHelp() {
    HelpFormatter hf(options());
    hf.setCommand(commandName());
    hf.setUsage("OPTIONS");
    hf.setHeader("LTE: Lua Task Engine.");
    hf.format(std::cout);
}

void LTE::Application::displayVersion() {
    std::cout << "LTE: Lua Task Engine: " << VERSION_STRING << std::endl;
}

void LTE::Application::defineOptions(OptionSet& options) {
    umask(S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);

    ServerApplication::defineOptions(options);

    RegExpValidator *pValidator;

    pValidator = new RegExpValidator("((?i)true|false)");
    options.addOption(
        Option("dns", "", "dns=false turns off DNS resolution (generally only used if communication is through a proxy server)")
            .required(false)
            .repeatable(false)
            .argument("true/false")
            .validator(pValidator)
            .binding("dns"));

    pValidator = new RegExpValidator("((?i)true|false)");
    options.addOption(
        Option("enable-docker-inspection", "", "tells the grid to enable/disable docker inspection when agent launches")
            .required(false)
            .repeatable(false)
            .argument("true/false")
            .validator(pValidator)
            .binding("enable-docker-inspection"));

    pValidator = new RegExpValidator("((?i)true|false)");
    options.addOption(
        Option("read-only", "", "read-only=true prevents the server from instructing the agent to make changes on the server")
            .required(false)
            .repeatable(false)
            .argument("true/false")
            .validator(pValidator)
            .binding("read-only"));

    options.addOption(
        Option("noproxy", "", "turn off proxy support")
            .group("proxy")
            .required(false)
            .repeatable(false)
            .binding("noproxy"));

    options.addOption(
        Option("proxy", "", "proxy IP/FQDN:port")
            .group("proxy")
            .required(false)
            .repeatable(false)
            .argument("value", true)
            .binding("proxy"));

    options.addOption(
        Option("proxy-user", "", "proxy username")
            .required(false)
            .repeatable(false)
            .argument("value")
            .binding("proxy-user"));

    options.addOption(
        Option("proxy-password", "", "proxy password")
            .required(false)
            .repeatable(false)
            .argument("value")
            .binding("proxy-password"));

    options.addOption(
        Option("config", "", "read configuration from file")
            .required(false)
            .repeatable(false)
            .argument("file")
            .binding("config"));

    pValidator = new RegExpValidator("[-._A-Za-z]{1,100}");
    options.addOption(
        Option("server-label", "", "optional server label for first time start")
            .required(false)
            .repeatable(false)
            .argument("value")
            .validator(pValidator)
            .binding("server-label"));

    pValidator = new RegExpValidator("[-._A-Za-z]{1,100}");
    options.addOption(
        Option("tag", "", "optional tag for first time start")
            .required(false)
            .repeatable(false)
            .argument("value")
            .validator(pValidator)
            .binding("tag"));

    options.addOption(
        Option("agent-key", "", "customer's agent key required for first time start")
            .required(false)
            .repeatable(false)
            .argument("value")
            .binding("agent-key"));

    options.addOption(
        Option("daemon-key", "", "customer's daemon key required for first time start")
            .required(false)
            .repeatable(false)
            .argument("value")
            .binding("daemon-key"));

    options.addOption(
        Option("idfile", "", "path to the external id file")
            .required(false)
            .repeatable(false)
            .argument("file")
            .binding("idfile"));

    options.addOption(
        Option("log", "", "path to the log file")
            .required(false)
            .repeatable(false)
            .argument("file")
            .binding("log"));

    options.addOption(
        Option("enable-log-rotation", "", "enable lofile rotation")
            .group("log-rotation")
            .required(false)
            .repeatable(false));


    options.addOption(
        Option("disable-log-rotation", "", "disable lofile rotation")
            .group("log-rotation")
            .required(false)
            .repeatable(false));

    pValidator = new RegExpValidator("^(100|[1-9][0-9]?)$");
    options.addOption(
        Option("max-log-size", "", "maximum size of the log file in Mb")
            .required(false)
            .repeatable(false)
            .argument("value")
            .validator(pValidator)
            .binding("max-log-size"));

    pValidator = new RegExpValidator("^(100|[1-9][0-9]?)$");
    options.addOption(
        Option("max-log-files", "", "maximum number of log files for log rotation")
            .required(false)
            .repeatable(false)
            .argument("value")
            .validator(pValidator)
            .binding("max-log-files"));

    options.addOption(
        Option("store", "", "path to the store file")
            .required(false)
            .repeatable(false)
            .argument("file")
            .binding("store"));


    options.addOption(
        Option("debug", "", "log debugging information")
            .required(false)
            .repeatable(false));

    options.addOption(
        Option("version", "", "print version number and exit")
            .required(false)
            .repeatable(false));

    options.addOption(
        Option("help", "", "display help information on command line arguments")
            .required(false)
            .repeatable(false));

    options.addOption(
        Option("grid", "", "URI of the grid processor to use")
            .required(false)
            .repeatable(false)
            .argument("value")
            .binding("grid"));
}

void LTE::Application::handleOption(const std::string& name, const std::string& value) {
    Poco::Logger&    logger = Poco::Util::Application::instance().logger();

    logger.information(name);
    logger.information(value);

    if (name == "help") {
        help = true;

    } else if (name == "version") {
        version = true;

    } else if (name == "debug") {
        Poco::Util::Application::instance().config().setBool("debug", true);

    } else if (name == "log") {
        std::string dir("log");
        log_file = Utils::daemonFile(dir, value);

    } else if (name == "store") {
        std::string dir("data");
        store_file = Utils::daemonFile(dir, value);

    } else if (name == "idfile") {
        std::string dir("data");
        log_file = Utils::daemonFile(dir, value);

    } else if (name == "enable-log-rotation") {
        Poco::Util::Application::instance().config().setBool("enable-log-rotation", true);

    } else if (name == "disable-log-rotation") {
        Poco::Util::Application::instance().config().setBool("disable-log-rotation", true);

    } else if (name == "max-log-size") {
        int logSize = std::stoi(value);
        Poco::Util::Application::instance().config().setInt("max-log-size", logSize);

    } else if (name == "max-log-files") {
        int logFiles = std::stoi(value);
        Poco::Util::Application::instance().config().setInt("max-log-files", logFiles);
    }

    return Poco::Util::Application::handleOption(name, value);
}


int LTE::Application::checkFDs(int maxfd) {
    struct stat     statbuf;
    struct rlimit   rlimits;
    int             fd = -1;

    Poco::Logger&    logger = Poco::Util::Application::instance().logger();

    if (fstat(0, &statbuf) == -1) {
        fd = 0;
    }

    if (fstat(1, &statbuf) == -1) {
        fd = 1;
    }

    if (fstat(2, &statbuf) == -1) {
        fd = 2;
    }

    if (fstat(3, &statbuf) == -1) {
        fd = 3;
    }

    if (fd != -1) {
        poco_fatal_f1(logger, "required fd %i is not open", fd);
        return Poco::Util::Application::EXIT_OSERR;
    }

    if (getrlimit(RLIMIT_NOFILE, &rlimits) == -1) {
        poco_fatal(logger, "getrlimit() failed");
        return Poco::Util::Application::EXIT_OSERR;
    }

    if (rlimits.rlim_cur > maxfd) {
        for (int i = maxfd + 1; i < rlimits.rlim_cur ; i++) {
            if (fstat(i, &statbuf) != -1) {
                poco_warning_f1(logger, "fd %i is unexpectedly open - closing", i);
                close(maxfd);
            }
        }
    }

    return Poco::Util::Application::EXIT_OK;
}


void LTE::Application::exit(int code) {
    exit(code); 
}


int LTE::Application::main(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cout << "Unrecognized command line arguments" << std::endl;
        Poco::Util::Application::instance().logger().fatal("Unrecognized command line arguments");
        ServerApplication::terminate();
    }

    if (help) {
        displayHelp();
        return Poco::Util::Application::EXIT_OK;
    }

    if (version) {
        displayVersion();
        return Poco::Util::Application::EXIT_OK;
    }

    if (config().getBool("application.runAsDaemon", false)) {
        int maxfd = controlled ? 6: 4;
        int rc = checkFDs(maxfd);
        if ( rc != Poco::Util::Application::EXIT_OK) {
            return rc;
        }
    }


    Store::deleteExpireRestartKeys();

    Poco::TaskManager tm;
    tm.start(new MainLoop);
    ServerApplication::waitForTerminationRequest();

    tm.cancelAll();
    tm.joinAll();

    return Poco::Util::Application::EXIT_OK;
}

std::string LTE::Application::getDaemonKey(void) {
    std::string s;
    if (config().has("daemon_key")) {
        s = config().getString("daemon_key");
    } else {
        if (!config().has("agent_key")) {
            logger().fatal("agent-key required for agent registration with core");
            terminate();
        }

        s = config().getString("agent_key");
    }

    return s;
}