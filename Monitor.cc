
#include "Poco/Util/Application.h"
#include "Poco/DateTimeFormatter.h"

#include "Application.h"
#include "Monitor.h"
#include "Core.h"

using Poco::Util::Application;
using Poco::Util::TimerTask;
using Poco::Util::Timer;

// Define the static variable.
Timer Monitor::monitor_timer;

void Monitor::Task::run(void) {
    LTE::Application& app = dynamic_cast<LTE::Application&>(Poco::Util::Application::instance());

    app.logger().information("Monitoring communication health");

    if (Core::stalled()) {
        app.logger().information("Communication has apparently stalled, exiting");
        app.exit(Poco::Util::Application::EXIT_SOFTWARE);
    }
}

void Monitor::schedule(void) {
    TimerTask::Ptr pTask = new Monitor::Task; 

    monitor_timer.schedule(pTask, 10 * 1000, 5 * 1000);
}
