#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"

using Poco::Util::Timer;
using Poco::Util::TimerTask;

class Monitor {
public:
    class Task: public TimerTask {
    public:
        void run();
    };

    static void schedule(void);
    static Timer monitor_timer; // just declare

};

#endif /* __MONITOR_H__ */
