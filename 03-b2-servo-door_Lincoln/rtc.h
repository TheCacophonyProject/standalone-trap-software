#ifndef RTC_h_
#define RTC_h_

#include <RTClib.h>

class RTC {
    public:
        void setup();
        void init();
        void setTime();
        DateTime getDateTime();
        bool isInActiveWindow(bool);
        RTC_DS1307 rtc;

};

#endif
