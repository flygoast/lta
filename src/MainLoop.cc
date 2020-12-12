#include <sys/types.h>
#include <unistd.h>

#include "Poco/Logger.h"
#include "Poco/DateTimeFormatter.h"

#include "Application.h"
#include "MainLoop.h"
#include "Monitor.h"
#include "Facts.h"
#include "GenericTask.h"
#include "DatabaseManager.h"
#include "Version.h"


using Poco::DateTimeFormatter;


void MainLoop::runTask(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    poco_information_f1(app.logger(), "LTE version %s starting up ...", std::string(VERSION_STRING));

    if (initialize()) {
        app.logger().information("Staring main loop ...");

        startScheduledTasks();

        while (!isCancelled()) {
            long interval = 1000 * app.config().getInt("heartbeat_interval", 1); 

            GenericTask::heartbeat();

            sleep(interval);

            if (!app.controlled || getppid() != 1) {
                continue;
            }

            app.logger().information("Parent process died.");

            cancel();

            if (isCancelled()) {
                break;
            }
        }

        GenericTask::shutdown();
    }
}


bool MainLoop::initialize(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    bool ret = false;

    while (!isCancelled()) {
        ret = GenericTask::initialize();
        if (ret) {
            break;
        }

        app.logger().error("Failed to initialize. Will retry...");
        sleep(30000);
    }

    return ret;
}


void MainLoop::startScheduledTasks(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());
    app.logger().information("Starting scheduling.");

    Monitor::schedule();
    Facts::schedule();
}
