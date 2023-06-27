#include "SC_Poll_Trigger.h"

SC_Poll_Trigger::SC_Poll_Trigger():Tool(){}


bool SC_Poll_Trigger::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_Trigger::Execute()
{
    if(m_data->SCMonitor.SumRelays == true)
    {
        //Get Triggerboard sensor
        m_data->SCMonitor.Trig0_mon = m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref); 
        m_data->SCMonitor.Trig1_mon = m_data->CB->GetTriggerDac1(m_data->SCMonitor.TrigVref);  
    }else
    {
        //Triggerboard
        m_data->SCMonitor.Trig0_mon = -1.0; 
        m_data->SCMonitor.Trig1_mon = -1.0;  
    }

    //Get Errorscodes
    vector<unsigned int> tmpERR = m_data->CB->returnErrors(); 
    m_data->SCMonitor.errorcodes.insert(std::end(m_data->SCMonitor.errorcodes), std::begin(tmpERR), std::end(tmpERR));
    tmpERR.clear();
    m_data->CB->clearErrors();

    return true;
}


bool SC_Poll_Trigger::Finalise()
{
    return true;
}
