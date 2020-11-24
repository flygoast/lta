#ifndef __LUA_H__
#define __LUA_H__

#include <iostream>

#include "Poco/Util/TimerTask.h"
#include "Poco/Util/Timer.h"
#include "Poco/Timestamp.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


using Poco::Util::TimerTask;
using Poco::Util::Timer;
using Poco::Timestamp;
using Poco::Mutex;

class Lua {
public:
    class LibraryManagement {
    public:
        
        void addLibrary();

        std::string library;
        static Mutex mutex;
    };

    class Interpreter {
    public:
        Interpreter(char todo);
        ~Interpreter(void);
        void setGlobalPointer(std::string& key, void *p);
        void setGlobalString(std::string& key, std::string& value);
        bool execute(std::string& script, std::string& arg, std::string* ret);
        bool execute(std::string& script, std::string& arg, std::string* ret_str, bool *ret_bool);
        bool execute(std::string& script, std::string& arg1, std::string& arg2, std::string* ret_str, bool *ret_bool);

    private:
        lua_State          *L;
        Poco::Timestamp     timestamp;

    };

    class Task: public TimerTask {
    public:
        Task(std::string script);
        void run(void);
        bool terminate(void);

        std::string         buffer;
        std::string         arg1;
        Poco::Timestamp     timestamp;
        Interpreter        *pInterpreter;
        std::string         command_id;

        bool                needBoolValue;
    };

    static void schedule(TimerTask::Ptr pTask);
    static Timer lua_timer; // just declare
};

#endif /* __LUA_H__ */
