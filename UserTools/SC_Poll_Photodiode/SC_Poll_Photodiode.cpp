#include "SC_Poll_Photodiode.h"

SC_Poll_Photodiode::SC_Poll_Photodiode():Tool(){}


bool SC_Poll_Photodiode::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_Photodiode::Execute()
{
    if(m_data->SCMonitor.SumRelays == true)
    {
        //Get Photodiode sensor
        m_data->SCMonitor.light = m_data->CB->GetPhotodiode();
    }else
    {
        m_data->SCMonitor.light = -1.0;
    }


    return true;
}


bool SC_Poll_Photodiode::Finalise()
{
    return true;
}
