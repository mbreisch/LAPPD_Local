#include "SC_Poll_Relays.h"

SC_Poll_Relays::SC_Poll_Relays():Tool(){}


bool SC_Poll_Relays::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_Relays::Execute()
{
    //Get Relay state
    m_data->SCMonitor.relayCh1_mon = m_data->CB->GetRelayState(1); 
    m_data->SCMonitor.relayCh2_mon = m_data->CB->GetRelayState(2); 
    m_data->SCMonitor.relayCh3_mon = m_data->CB->GetRelayState(3);

    m_data->SCMonitor.SumRelays = m_data->SCMonitor.relayCh1_mon;

    return true;
}


bool SC_Poll_Relays::Finalise()
{
    return true;
}
