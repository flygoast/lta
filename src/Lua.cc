#include "Poco/Util/Application.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/AutoPtr.h"


#include "Lua.h"
#include "string.h"

using Poco::Util::Application;
using Poco::DateTimeFormatter;
using Poco::Util::TimerTask;


static void termination_hook(lua_State *L, lua_Debug *ar);


// Static variables defined in header files.
Timer Lua::lua_timer;
Mutex Lua::LibraryManagement::mutex;


Lua::Task::Task(std::string script) {
    buffer = script;
}

void Lua::Task::run(void) {
    Application& app = Application::instance();

    app.logger().information("running task" + Poco::DateTimeFormatter::format(app.uptime())); 
    app.logger().information("Lua Task " + buffer);

    buffer = "function main() if true then print(\"hello lua2 from cpp\") end return \"hello\" end";

    pInterpreter = new Interpreter(1);

    std::string key("__agent_command_id__");
    // TODO
    command_id = "111111";
    pInterpreter->setGlobalString(key, command_id);
    key = "__agent_task_pointer__";
    pInterpreter->setGlobalPointer(key, this);

    bool execution_success = false;

    std::string ret_str;

    if (needBoolValue) {
        bool ret_bool = true;
        // TODO:: decrypt lua content
        execution_success = pInterpreter->execute(buffer, arg1, &ret_str, &ret_bool);
    } else {
        execution_success = pInterpreter->execute(buffer, arg1, &ret_str);
    }

    if (execution_success) {
        app.logger().information("task finished");
    } else {
        app.logger().error("task failed");
    }

    delete pInterpreter;
}

bool Lua::Task::terminate() {
    return true;
}


void Lua::schedule(TimerTask::Ptr pTask) {
    Poco::Timestamp ts;
    lua_timer.schedule(pTask, ts);
}


//=============================================
// Lua Interpreter
//=============================================
Lua::Interpreter::Interpreter(char todo) {
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_sethook(L, ::termination_hook, LUA_MASKCOUNT, 10000);

    //luaopen_posix(L);
    // TODO
    //add_loader();
    timestamp.update();
}

Lua::Interpreter::~Interpreter(void) {
    lua_close(L);
    std::cout << "~~~~~~~" << std::endl;
}

void Lua::Interpreter::setGlobalPointer(std::string& key, void *p) {
    lua_pushlightuserdata(L, p);
    lua_setglobal(L, key.c_str());

}

void Lua::Interpreter::setGlobalString(std::string& key, std::string& value) {
    lua_pushstring(L, value.c_str());
    lua_setglobal(L, key.c_str());

}

bool Lua::Interpreter::execute(std::string& script, std::string& arg, std::string* ret) {
    const char *err;
    int len;
    if (luaL_loadbuffer(L, script.c_str(), script.length(), "script") || lua_pcall(L, 0, 0, 0)) {
        err = (char *) lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    lua_getglobal(L, "main");
    int type = lua_type(L, -1);
    if (type == LUA_TNIL) {
        err = "lua function 'main' not found";
        len = strlen(err);
        ret->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    lua_pushstring(L, arg.c_str());
    if (lua_pcall(L, 1, 1, 0) != 0) {
        err = lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    if (!lua_isstring(L, -1)) {
        err = "lua function return value mut be of type string";
        len = strlen(err);
        ret->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    const char *result = lua_tolstring(L, -1, NULL);
    len = strlen(result);
    ret->assign(result, len);
    lua_settop(L, -2);

    return true;
}

bool Lua::Interpreter::execute(std::string& script, std::string& arg, std::string* ret_str, bool *ret_bool) {
    const char *err;
    int len;
    if (luaL_loadbuffer(L, script.c_str(), script.length(), "script") || lua_pcall(L, 0, 0, 0)) {
        err = (char *) lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    *ret_bool = true;

    lua_getglobal(L, "main");
    int type = lua_type(L, -1);
    if (type == LUA_TNIL) {
        err = "lua function 'main' not found";
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    lua_pushstring(L, arg.c_str());

    // Lua main() function would return a string and a boolean value. 
    if (lua_pcall(L, 1, 2, 0) != 0) {
        err = lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    if (lua_isstring(L, -1) && lua_type(L, -2) == LUA_TBOOLEAN) {
        const char *value = lua_tolstring(L, -1, NULL);
        len = strlen(value);
        ret_str->assign(value, len);
        int b = lua_toboolean(L, -2);
        *ret_bool = (b != 0);
        lua_settop(L, -3);
        return true;
    } else {
        err = "lua scan return values must be of type boolean, string";
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -3);
        return false;
    }
    return false;
}

bool Lua::Interpreter::execute(std::string& script, std::string& arg1, std::string& arg2, std::string* ret_str, bool *ret_bool) {
    const char *err;
    int len;
    if (luaL_loadbuffer(L, script.c_str(), script.length(), "script") || lua_pcall(L, 0, 0, 0)) {
        err = (char *) lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    *ret_bool = true;

    lua_getglobal(L, "main");
    int type = lua_type(L, -1);
    if (type == LUA_TNIL) {
        err = "lua function 'main' not found";
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    lua_pushstring(L, arg1.c_str());
    lua_pushstring(L, arg2.c_str());

    // Lua main() function would return a string and a boolean value. 
    if (lua_pcall(L, 2, 2, 0)) {
        err = lua_tolstring(L, -1, NULL);
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -2);
        return false;
    }

    if (lua_isstring(L, -1) && lua_type(L, -2) == LUA_TBOOLEAN) {
        const char *value = lua_tolstring(L, -1, NULL);
        len = strlen(value);
        ret_str->assign(value, len);
        int b = lua_toboolean(L, -2);
        *ret_bool = (b != 0);
        lua_settop(L, -3);
        return true;
    } else {
        err = "lua scan return values must be of type boolean, string";
        len = strlen(err);
        ret_str->assign(err, len);
        lua_settop(L, -3);
        return false;
    }
    return false;
}

static void termination_hook(lua_State *L, lua_Debug *ar) {
    Lua::Task *pTask;
    lua_getglobal(L, "__agent_task_pointer__");
    pTask = (Lua::Task *)lua_touserdata(L, -1);
    if (pTask) {
        if (pTask->terminate()) {
            lua_pushstring(L, "terminated");
            lua_error(L);
        }
    }
}
