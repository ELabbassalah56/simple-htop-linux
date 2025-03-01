#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) 
{   
    //elapsed time is 0
    if(seconds == 0)
        return "00:00:00";
    // elapsed time is less than 1 minute
    else if (seconds<60)
        return "00:00:"+std::to_string(seconds);

    // caluclate the time elapsed in hours, minutes and seconds
    long hours = seconds/3600;
    long minutes = (seconds%3600)/60;
    long sec = (seconds%3600)%60;
    // return the elapsed time in HH:MM:SS format
    return std::to_string(hours)+":"+std::to_string(minutes)+":"+std::to_string(sec);
}