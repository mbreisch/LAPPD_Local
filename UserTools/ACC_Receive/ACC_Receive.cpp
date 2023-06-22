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
    RunEndConfigType="local";
    m_variables.Get("RunStartConfigType",RunStartConfigType);
    m_variables.Get("RunEndConfigType",RunEndConfigType);
    
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
        printf("ACC_receive initialise");
        running = true;
    
        Store tmp;
        if(m_data->running)
        {
            if(RunStartConfigType=="local"){ tmp.Initialise(RunStartConfigName);}
        }

        if(m_verbose>1){tmp.Print();}
        std::cout<<" did it initiailiase? = "<<m_data->config.Initialise(&tmp)<<std::endl;
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
            }
        }
    }
  
    return true;
}


bool ACC_Receive::Finalise()
{
    return true;
}
