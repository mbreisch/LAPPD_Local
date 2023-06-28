#include "ACC_Receive.h"
#include <sstream>

ACC_Receive::ACC_Receive():Tool(){}

bool ACC_Receive::Initialise(std::string configfile, DataModel &data)
{
    m_data= &data;
    m_log= m_data->Log;

    // allow overrides from local config file
    localconfigfile=configfile;
    if(configfile!="")  m_variables.Initialise(configfile);
    
    // get Run[Start/End]Configs
    RunStartConfigType="local";
    m_variables.Get("RunStartConfigType",RunStartConfigType);
    m_variables.Get("RunStartConfigName",RunStartConfigName);
    
    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
    if(m_verbose>2) m_variables.Print();
    
    running=false;
    
    return true;
}


bool ACC_Receive::Execute()
{
    // on start or end of run, update ACC configuration
    if(!running)
    {
        running = true;
    
        Store tmp;
        if(RunStartConfigType=="local"){ tmp.Initialise(RunStartConfigName);}

        if(m_verbose>2){tmp.Print();}
        std::cout<<" Did it Initiailiase? = "<<m_data->config.Initialise(&tmp)<<std::endl<<std::endl;
        m_data->config.Print();
        std::string choice_yn;
        while(true)
        {
            std::cout << "Are you ok with these settings (y/n)?   ";
            std::cin >> choice_yn;
            if(choice_yn=="y")
            {
                break;
            }else if(choice_yn=="n")
            {
                m_variables.Set("StopLoop",1);
                break;
            }
        }
    }
  
    return true;
}


bool ACC_Receive::Finalise()
{
    return true;
}
