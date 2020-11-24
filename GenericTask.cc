#include "Poco/Util/Application.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Random.h"

#include "GenericTask.h"
#include "Lua.h"

using Poco::Util::Application;
using Poco::Util::TimerTask;
using Poco::DateTimeFormatter;

void GenericTask::heartbeat(void) {
    reportMessages();

    std::string command = "script";

    executeCommand(command);
}

void GenericTask::executeCommand(std::string& command) {
    //TODO
    Application& app = Application::instance();

    if (command == "script") {
        processScript(command);
        return;
    }

    if (command == "shutdown") {
        return;
    }

    if (command == "update_configuration") {
        return;
    }

    if (command == "restore_initial_configuration") {
        return;
    }

    app.logger().error("execute command failed " + command + Poco::DateTimeFormatter::format(app.uptime())); 
}

void GenericTask::reportMessages() {
    // TODO
}

void GenericTask::processScript(std::string &command) {

    std::string command_type = "execute";

    if (command_type == "lib") {
        processLibraryScript(command);
        return;
    }

    if (command_type == "execute") {
        processExecutableScript(command);
        return;
    }
}

void GenericTask::processLibraryScript(std::string& command) {
    // TODO
}

void GenericTask::processExecutableScript(std::string& command) {
    TimerTask::Ptr pTask = new Lua::Task("print 1"); 
    Lua::schedule(pTask);
}


bool GenericTask::initialize(void) {
    return true;
}

void GenericTask::shutdown(void) {
}
