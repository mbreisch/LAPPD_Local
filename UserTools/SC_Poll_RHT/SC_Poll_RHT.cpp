#include "SC_Poll_RHT.h"

SC_Poll_RHT::SC_Poll_RHT():Tool(){}


bool SC_Poll_RHT::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    return true;
}


bool SC_Poll_RHT::Execute()
{
    bool retchk;
    if(m_data->SCMonitor.SumRelays == true)
    {
        //Get Hum/Temp sensor data
        vector<float> RHT = m_data->CB->GetTemp(); 
        m_data->SCMonitor.temperature_mon = RHT[0];
        m_data->SCMonitor.humidity_mon = RHT[1];
        RHT.clear();

        //Realise Error check
        retchk = TEMPCHK();
        if(retchk==false)
        {
            //report error behavior 
            m_data->SCMonitor.errorcodes.push_back(0xCC01EE00);
        }
        retchk = HUMIDITYCHK();
        if(retchk==false)
        {
            //report error behavior
            m_data->SCMonitor.errorcodes.push_back(0xCC01EE01);
        } 
    }else
    {
        m_data->SCMonitor.temperature_mon = -1.0;
        m_data->SCMonitor.humidity_mon = -1.0;
    }

    return true;
}


bool SC_Poll_RHT::Finalise()
{
    return true;
}


bool SC_Poll_RHT::TEMPCHK(){
    int retval=-2;
    if(m_data->SCMonitor.temperature_mon < m_data->SCMonitor.LIMIT_temperature_low)
    {
        m_data->SCMonitor.FLAG_temperature = 0;
        return true;
    }else if(m_data->SCMonitor.temperature_mon >= m_data->SCMonitor.LIMIT_temperature_low && m_data->SCMonitor.temperature_mon < m_data->SCMonitor.LIMIT_temperature_high)
    {
        m_data->SCMonitor.FLAG_temperature = 1;
        return true;
    }else if(m_data->SCMonitor.temperature_mon >= m_data->SCMonitor.LIMIT_temperature_high)
    {
        bool ret;
        bool safety=true;

        ret = HardShutdown(4.1);
        if(ret==false){safety=false;}

        m_data->SCMonitor.FLAG_temperature = 2;

        if(safety==false){m_data->SCMonitor.FLAG_temperature = 3;}

        return safety; 
    }else
    {
        return false;
    }
}


bool SC_Poll_RHT::HUMIDITYCHK(){
    int retval=-2;
    tool_humidity_limit = 0.0;

    //Adjust humidity value based on state
    if(m_data->SCMonitor.temperature_mon<=25)
    {
        tool_humidity_limit = 2.5*m_data->SCMonitor.LIMIT_humidity_high;
    }else if(m_data->SCMonitor.temperature_mon>25 && m_data->SCMonitor.temperature_mon<=35)
    {
        float lina = -1.5*m_data->SCMonitor.LIMIT_humidity_high/10;
        float linb = 6.25*m_data->SCMonitor.LIMIT_humidity_high;
        tool_humidity_limit = lina*m_data->SCMonitor.temperature_mon+linb;
    }else if(m_data->SCMonitor.temperature_mon>35 && m_data->SCMonitor.temperature_mon<=45)
    {
        tool_humidity_limit = m_data->SCMonitor.LIMIT_humidity_high;
    }else if(m_data->SCMonitor.temperature_mon>45)
    {
	    tool_humidity_limit = 0.8*m_data->SCMonitor.LIMIT_humidity_high;
    }else
    {
        std::cout<<"Error! Input not possible"<<std::endl;
    }

    if(m_data->SCMonitor.humidity_mon < m_data->SCMonitor.LIMIT_humidity_low)
    {
        m_data->SCMonitor.FLAG_humidity = 0;
        return true;
    }else if(m_data->SCMonitor.humidity_mon >= m_data->SCMonitor.LIMIT_humidity_low && m_data->SCMonitor.humidity_mon < tool_humidity_limit)
    {
        m_data->SCMonitor.FLAG_humidity = 1;
        return true;
    }else if(m_data->SCMonitor.humidity_mon >= tool_humidity_limit)
    {
        bool ret;
        bool safety=true;

        ret = HardShutdown(4.2);
        if(ret==false){safety=false;}

        m_data->SCMonitor.FLAG_humidity = 2;

        if(safety==false){m_data->SCMonitor.FLAG_humidity = 3;}

        return safety; 
    }else
    {
        return false;
    }
}  


bool SC_Poll_RHT::HardShutdown(float errortype)
{
    bool retbool = true;
    for(int relay=1; relay<=3; relay++)
    {
        int tries = 0;
        int retval = -2;
        int max_tries = 50;
        retbool = true;
        //Instant shutdown
        while(retval!=0 && tries<max_tries)
        {
            retval = m_data->CB->SetRelay(relay,false); 
            tries++;
        }
        if(relay==1){m_data->SCMonitor.SumRelays = false;}

        if(retval!=0 && tries>=max_tries)
        {
            m_data->SCMonitor.errorcodes.push_back((0xCC01EE00 | (int)((errortype-4)*10)));
            retbool = false;
            if(relay==1){m_data->SCMonitor.SumRelays = true;}
        }
    }

    std::fstream errfile("./HardShutdownList.txt", std::ios_base::out | std::ios_base::app);
    errfile << "System has shut down in emergency mode at " <<  m_data->SCMonitor.timeSinceEpochMilliseconds << " ms since epoch" << std::endl;
    if(errortype==4.1)
    {
        errfile << "It shut down in the Temperature tool due to a read value of: " << m_data->SCMonitor.temperature_mon << " with the limit being >=" << m_data->SCMonitor.LIMIT_temperature_high << std::endl;
    }else if(errortype==4.2)
    {
       errfile << "It shut down in the Humidity tool due to a read value of: " << m_data->SCMonitor.humidity_mon << " with the limit being >=" << tool_humidity_limit << std::endl; 
    }else
    {
        errfile << "It shut down in the Temperature tool due to a read value of: " << m_data->SCMonitor.temperature_mon << " with the limit being >=" << m_data->SCMonitor.LIMIT_temperature_high << std::endl;
        errfile << "It shut down in the Humidity tool due to a read value of: " << m_data->SCMonitor.humidity_mon << " with the limit being >=" << tool_humidity_limit << std::endl; 
    }
    errfile << "Now printing all available values available. All values read after this tool will be from the last read cycle!" << std::endl;
    errfile << "LAPPD ID is " << m_data->SCMonitor.LAPPD_ID << std::endl;
	errfile << "Timestamp ms since epoch = " << m_data->SCMonitor.timeSinceEpochMilliseconds << std::endl;
	errfile << "humidity = " << m_data->SCMonitor.humidity_mon << std::endl;
	errfile << "temperature = " << m_data->SCMonitor.temperature_mon << std::endl;
	errfile << "thermistor = " << m_data->SCMonitor.temperature_thermistor << std::endl;
	errfile << "HV state should be " << std::boolalpha << m_data->SCMonitor.HV_state_set << " and is " << std::boolalpha << m_data->SCMonitor.HV_mon << " and is set to " << m_data->SCMonitor.HV_volts << " V" << std::endl;
	errfile << "Returned HV is " << m_data->SCMonitor.HV_return_mon << "V" << std::endl;
	errfile << "LV state should be " << std::boolalpha << m_data->SCMonitor.LV_state_set << " and is " << std::boolalpha << m_data->SCMonitor.LV_mon << std::endl;
	errfile << "LV voltages are V(3.3)= " << m_data->SCMonitor.v33 << "V, V(3.1)= " << m_data->SCMonitor.v25 << "V, V(1.8)= " << m_data->SCMonitor.v12 << "V" << std::endl;	
	errfile << "Temperature warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_temperature << std::endl;
	errfile << "Humidity warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_humidity << std::endl;
	errfile << "Temperature 2 warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_temperature_Thermistor << std::endl;
	errfile << "Saltbridge warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_saltbridge << std::endl;
	errfile << "Relay 1 is after off " << std::boolalpha << m_data->SCMonitor.relayCh1_mon << std::endl;
	errfile << "Relay 2 is after off " << std::boolalpha << m_data->SCMonitor.relayCh2_mon << std::endl;
	errfile << "Relay 3 is after off " << std::boolalpha << m_data->SCMonitor.relayCh3_mon << std::endl;
	errfile << "Threshold for DAC 0 is " << m_data->SCMonitor.Trig0_mon << " V" << std::endl;
	errfile << "Threshold for DAC 1 is " << m_data->SCMonitor.Trig1_mon << " V" << std::endl;
	errfile << "Photodiode return is " << m_data->SCMonitor.light << std::endl;
	errfile << "Saltbridge return is " << m_data->SCMonitor.saltbridge << std::endl;
	if(m_data->SCMonitor.errorcodes.size()==1 && m_data->SCMonitor.errorcodes[0]==0x00000000)
	{
		printf("No errorcodes found all good: 0x%08x\n", m_data->SCMonitor.errorcodes[0]);
	}else
	{
		printf("Errorcodes found: %li\n", m_data->SCMonitor.errorcodes.size());
        if(m_data->SCMonitor.errorcodes.size()<100)
        {            
            for(unsigned int k: m_data->SCMonitor.errorcodes)
            {
                printf("0x%08x\n",k);	
            }
        }
	}
    errfile << "-------------" << std::endl; 
    errfile.close();

    return retbool;
}
