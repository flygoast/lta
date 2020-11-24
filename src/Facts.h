#ifndef __FACTS_H__
#define __FACTS_H__

#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTask.h"

using Poco::Util::Timer;
using Poco::Util::TimerTask;

class Facts {
public:
    class Task: public TimerTask {
    public:
        void run();
    };

    static void schedule(void);
    static Timer facts_timer; // just declare

};

#endif /* __FACTS_H__ */
