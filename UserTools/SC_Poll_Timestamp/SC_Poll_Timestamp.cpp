#include "SC_Poll_Timestamp.h"

SC_Poll_Timestamp::SC_Poll_Timestamp():Tool(){}


bool SC_Poll_Timestamp::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_Timestamp::Execute()
{
    //Get Timestamp
    unsigned long long timeSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_data->SCMonitor.timeSinceEpochMilliseconds = convertToCST(timeSinceEpoch); 

    return true;
}


bool SC_Poll_Timestamp::Finalise()
{
    return true;
}

std::string SC_Poll_Timestamp::convertToCST(unsigned long long timestampInMs) 
{
    // Create a time_point from the Unix timestamp in milliseconds
    auto timePoint = std::chrono::system_clock::time_point(std::chrono::milliseconds(timestampInMs));

    // Convert the time_point to a time_t
    auto timeT = std::chrono::system_clock::to_time_t(timePoint);

    // Convert the time_t to a tm struct in the local timezone
    auto tmLocal = std::localtime(&timeT);

    // Check if daylight saving time is in effect
    bool isDST = (tmLocal->tm_isdst > 0);

    // Set the timezone to either CST or CDT, depending on DST
    const char* timezone = isDST ? "CDT" : "CST";

    // Convert the tm struct to a time_point in the local timezone
    auto tmOffset = std::chrono::hours(tmLocal->tm_gmtoff / 3600);
    auto timePointLocal = std::chrono::time_point_cast<std::chrono::seconds>(timePoint) + tmOffset;

    // Convert the time_point to a Unix timestamp in milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timePointLocal.time_since_epoch());
    return std::to_string(ms.count());
}

