
#include "Poco/Util/Application.h"
#include "Poco/DateTimeFormatter.h"

#include "json/json.h"

#include "Facts.h"
#include "Utils.h"
#include "Core.h"

using Poco::Util::Application;
using Poco::Util::TimerTask;
using Poco::Util::Timer;

// Define the static variable.
Timer Facts::facts_timer;

void Facts::Task::run(void) {
    Application& app = Application::instance();

    Json::Value root;
    root["ip_address"] = Utils::networkInterfaces();
    root["fqdn"] = Utils::fqdn();
    root["node_name"] = Utils::nodeName();
    root["firewall_present"] = Utils::firewallPresent();
    root["os_display_name"] = Utils::osDisplayName();
    root["machine"] = Utils::machine();
    root["created_at"] = Utils::utcNowStr();

    std::string facts = "facts";

    Core::call(facts, root);
}

void Facts::schedule(void) {
    TimerTask::Ptr pTask = new Facts::Task; 

    Application& app = Application::instance();

    long interval = 1000 * app.config().getInt("facts_interval", 5);
    facts_timer.scheduleAtFixedRate(pTask, 5 * 1000, interval);
}
