#include "SC_Poll_HV.h"

SC_Poll_HV::SC_Poll_HV():Tool(){}


bool SC_Poll_HV::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
    if(!m_variables.Get("PRINTFLAG",PRINTFLAG)) PRINTFLAG=0;

    return true;
}


bool SC_Poll_HV::Execute()
{
    bool retchk;
    if(m_data->SCMonitor.SumRelays == true)
    {
        //Get HV state
        m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();  

        //Get HV value
        m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;          

        if(m_data->SCMonitor.HV_mon==1)
        {
            retchk = HVCHK();
            if(retchk==false)
            {
                //report error behavior 
                m_data->SCMonitor.errorcodes.push_back(0xCC00EE00);
            }  	  
        }
    }else
    {
        m_data->SCMonitor.HV_mon = -1.0;
        m_data->SCMonitor.HV_return_mon = -1.0;
    }


    return true;
}


bool SC_Poll_HV::Finalise()
{
    int retval=-2;
    int counter;
    float down_voltage = 0;
    if(m_data->SCMonitor.HV_mon==1)
    {
        m_data->CB->SetHV_voltage(down_voltage,m_data->SCMonitor.HV_return_mon,0);

        m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF(); 
        m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue; 
        counter=0;
		while(fabs(m_data->SCMonitor.HV_return_mon-down_voltage)>50)
		{
			usleep(TIMESLEEP);
			m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
			m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
			if(counter>=30){break;}
			counter++;
		}
        if(m_data->SCMonitor.HV_return_mon>50)
        {
            m_data->CB->SetHV_voltage(down_voltage,m_data->SCMonitor.HV_return_mon,0);

            m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF(); 
            m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;
            counter=0;
            while(fabs(m_data->SCMonitor.HV_return_mon-down_voltage)>50)
            {
                usleep(TIMESLEEP);
                m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
                m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
                if(counter>=30){break;}
                counter++;
            }
        }
        retval = m_data->CB->SetHV_ONOFF(false);
        if(retval!=0 && retval!=1)
        {
            //std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;
            m_data->SCMonitor.errorcodes.push_back(0xCC00EE01);
        }
	    m_data->SCMonitor.HV_return_mon = down_voltage;
	    m_data->CB->get_HV_volts = m_data->SCMonitor.HV_return_mon;
	    std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
	    outfile << m_data->CB->get_HV_volts;
	    outfile.close();
    }

    m_data->CB->Disconnect();
    delete m_data->CB;
    m_data->CB=0;

    return true;
}


bool SC_Poll_HV::HVCHK()
{
    int retval=-2; 
    int counter=0;
    float timer=0.0;

    //Verbosity print for first appearance
    if(m_verbose>1)
    {
        std::cout << "HV set value was: " << m_data->SCMonitor.HV_volts << std::endl;	
        std::cout << "Last readback HV value before multi check was: " << m_data->SCMonitor.HV_return_mon << std::endl;	        
    }   

    //Multi check start
    if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>200)
    {
        timer = 0.0;
        while(timer<timeout) //timeout = 10s
        {
            m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
            m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;

            if(m_data->SCMonitor.HV_mon!=1){if(m_verbose>1){std::cout<<"HV state: "<<m_data->SCMonitor.HV_mon<<std::endl;}}

            if(m_verbose>1)
            {
                std::cout << "HV value after " << timer << " s was: " <<  m_data->SCMonitor.HV_return_mon << " V" << std::endl;
            }

            if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)<200)
            {              
                if(i_chk<1000 && PRINTFLAG==1)
                {
                    std::fstream outfile("./configfiles/SlowControl/HV_timer_list.txt", std::ios_base::out | std::ios_base::app);
                    outfile << m_data->SCMonitor.timeSinceEpochMilliseconds  << " : " << m_data->SCMonitor.HV_volts << "V -> after " << timer << "s" << std::endl;
                    outfile.close();
                    i_chk++;
                }
                break;
            }

            usleep(timestep*1000000);
            timer+=timestep;
        }
    }

    if(m_data->SCMonitor.HV_return_mon < (m_data->SCMonitor.HV_volts-200) || m_data->SCMonitor.HV_return_mon > (m_data->SCMonitor.HV_volts+200))
    {
        float down_voltage = 0;
        m_data->SCMonitor.errorcodes.push_back(0xCC00EE02);
        m_data->CB->SetHV_voltage(down_voltage,m_data->SCMonitor.HV_return_mon,2);
        
        m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF(); 
        m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue; 
        counter=0;
        while(fabs(m_data->SCMonitor.HV_return_mon-down_voltage)>50)
        {
            usleep(TIMESLEEP);
            m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
            m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
            if(counter>=50){break;}
            counter++;
        }  

        if(m_data->SCMonitor.HV_return_mon>50)
        {
            bool ret;
            bool safety=true;

            ret = HardShutdown(0);
            if(ret==false){safety=false;}

            m_data->SCMonitor.errorcodes.push_back(0xCC00EE03);
            return safety;
        }else
        {
            retval = m_data->CB->SetHV_ONOFF(false);
            if(retval!=0 && retval!=1)
            {
                //std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;
                m_data->SCMonitor.errorcodes.push_back(0xCC00EE04);
            }
            m_data->SCMonitor.HV_return_mon = down_voltage;
            m_data->CB->get_HV_volts = m_data->SCMonitor.HV_return_mon;
            std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
            outfile << m_data->CB->get_HV_volts;
            outfile.close();
            return true;
        }
    }else
    {
        return true;
    }
}


bool SC_Poll_HV::HardShutdown(int errortype)
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
            m_data->SCMonitor.errorcodes.push_back((0xCC00EE00 | errortype));
            retbool = false;
            if(relay==1){m_data->SCMonitor.SumRelays = true;}
        }
    }

    std::fstream errfile("./HardShutdownList.txt", std::ios_base::out | std::ios_base::app);
    errfile << "System has shut down in emergency mode at " <<  m_data->SCMonitor.timeSinceEpochMilliseconds << " ms since epoch" << std::endl;
    errfile << "It shut down in the HV tool due to a read value of: " << m_data->SCMonitor.HV_return_mon << " with the limit being 200V +- set value of" << m_data->SCMonitor.HV_volts << std::endl;
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