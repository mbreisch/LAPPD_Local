#include "SC_Poll_LV.h"

SC_Poll_LV::SC_Poll_LV():Tool(){}


bool SC_Poll_LV::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_LV::Execute()
{
    if(m_data->SCMonitor.SumRelays == true)
    {
        //Get LV state
        m_data->SCMonitor.LV_mon = m_data->CB->GetLV_ONOFF(); 

        //Get LV values
        std::vector<float> LVvoltage = m_data->CB->GetLV_voltage();  
        m_data->SCMonitor.v33 = LVvoltage[0];
        m_data->SCMonitor.v25 = LVvoltage[1];
        m_data->SCMonitor.v12 = LVvoltage[2];
        LVvoltage.clear();
    }else
    {
        m_data->SCMonitor.LV_mon = -1; 
        m_data->SCMonitor.v33 = -1.0;
        m_data->SCMonitor.v25 = -1.0;
        m_data->SCMonitor.v12 = -1.0;
    }


    return true;
}


bool SC_Poll_LV::Finalise()
{
    return true;
}
