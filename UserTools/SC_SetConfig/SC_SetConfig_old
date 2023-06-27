#include "SC_SetConfig.h"

SC_SetConfig::SC_SetConfig():Tool(){}


bool SC_SetConfig::Initialise(std::string configfile, DataModel &data){

	if(configfile!="")  m_variables.Initialise(configfile);
	//m_variables.Print();

	m_data= &data;
	m_log= m_data->Log;

	m_data->CB= new Canbus();

	std::fstream infile("./configfiles/SlowControl/LastHV.txt", std::ios_base::in);
	if(infile.is_open())
	{
		infile >> m_data->CB->get_HV_volts;
		infile.close();
	}
	
	string ThermistorID;
	m_variables.Get("ThermistorID",ThermistorID);
	m_data->CB->SetThermistorID(ThermistorID);

	if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

	m_variables.Get("LAPPD_ID",LAPPD_ID);
	m_data->SCMonitor.LAPPD_ID = LAPPD_ID;
	
	return true;
}


bool SC_SetConfig::Execute()
{
	//Set the LAPPD_ID
	m_data->SCMonitor.LAPPD_ID = LAPPD_ID;

	//check LV/HV state_set 
  	if(m_verbose>2){std::cout<<"In tool, before skip of EndRun"<<std::endl;}
  	if(m_data->SCMonitor.recieveFlag==0){return true;} //EndRun catch

	if(m_verbose>2){std::cout<<"Start Setup"<<std::endl;}
  	if(m_data->SCMonitor.recieveFlag==1)
	{
		//Pre get the relay states
		m_data->SCMonitor.relayCh1_mon = m_data->CB->GetRelayState(1);
		m_data->SCMonitor.SumRelays = m_data->SCMonitor.relayCh1_mon;

		if(m_data->SCMonitor.relayCh1_mon==0 && m_data->SCMonitor.relayCh1==1)//From off to on 
		{
			TurnOn();
		}else if(m_data->SCMonitor.relayCh1_mon==1 && m_data->SCMonitor.relayCh1==1)//From on to on 
		{
			Update();
		}else if(m_data->SCMonitor.relayCh1_mon==1 && m_data->SCMonitor.relayCh1==0)//From on to off 
		{
			TurnOff();
		}else
		{
			if(m_verbose>1){std::cout<<"OFF stays OFF"<<std::endl;}
		}	
	} //Normal Setup condition

	if(m_verbose>2){std::cout<<"In tool, before skip of Run"<<std::endl;}
	if(m_data->SCMonitor.recieveFlag==2){return true;} //After setup continous run mode

	if(m_verbose>2){std::cout<<"End of tool execute"<<std::endl;}
	return true;

}


bool SC_SetConfig::Finalise()
{
	m_data->CB->get_HV_volts = 0;
	return true;
}


bool SC_SetConfig::TurnOn()
{
	int counter=0;

	//Turn HV down
	if(m_data->SCMonitor.HV_volt!=0)
	{
		std::cout<<"Since relays will be powered down HV will be first set to 0"<<std::endl;
		m_data->SCMonitor.HV_volt = 0;
	}

	m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
	m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;
	
	if(m_data->SCMonitor.HV_volts!=m_data->SCMonitor.HV_return_mon)
	{
		retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
		if(retval==0)
		{	
			m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
			m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	

			counter = 0;
			while(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
			{
				usleep(10000000);
				m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
				m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
				if(counter>=30){break;}
				counter++;
			}
			if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
			{
				if(m_verbose>1){std::cout << "HV was: " << m_data->SCMonitor.HV_return_mon << std::endl;}
				m_data->SCMonitor.errorcodes.push_back(0xCB03EE04);
			}
			m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
			std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
			outfile << m_data->CB->get_HV_volts;
			outfile.close();
		}else
		{
			if(m_verbose>3){std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB03EE03);
		}
	}
	//Turn HV module off
	if(m_data->SCMonitor.HV_state_set!=0)
	{
		std::cout<<"Since relays will be powered down HV module will be switched off"<<std::endl;
		m_data->SCMonitor.HV_state_set = 0;
	}
	retval = m_data->CB->SetHV_ONOFF(m_data->SCMonitor.HV_state_set);
	if(retval!=0 && retval!=1)
	{
		if(m_verbose>3){std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;}
		m_data->SCMonitor.errorcodes.push_back(0xCB03EE01);
	}
	//Turn LV off
	if(m_data->SCMonitor.LV_state_set!=0)
	{
		std::cout<<"Since relays will be powered down LV module will be switched off"<<std::endl;
		m_data->SCMonitor.LV_state_set = 0;
	}
	retval = m_data->CB->SetLV(m_data->SCMonitor.LV_state_set);
	if(retval!=0 && retval!=1)
	{
		if(m_verbose>3){std::cout << " There was an error (Set LV) with retval: " << retval << std::endl;}
		m_data->SCMonitor.errorcodes.push_back(0xCB02EE01);
	}
	//Turn Relays off
	if(m_data->SCMonitor.relayCh1!=m_data->SCMonitor.relayCh1_mon)
	{
		if(m_verbose>3){std::cout << "Relay 1 is " << std::boolalpha << m_data->SCMonitor.relayCh1_mon << " and will be " << std::boolalpha << m_data->SCMonitor.relayCh1  << std::endl;}
		retval = m_data->CB->SetRelay(1,m_data->SCMonitor.relayCh1);
		if(retval!=0 && retval!=1)
		{
			if(m_verbose>3){std::cout << "There was an error (Relay 1) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB01EE01);
		}else
		{
			if(m_verbose>3){std::cout << "Very weird relay behavior! Got bool value other than 0/1!"<<stD::endl;}
		}
	}
	m_data->SCMonitor.relayCh1_mon = m_data->CB->GetRelayState(1);
	m_data->SCMonitor.SumRelays = m_data->SCMonitor.relayCh1_mon;

	m_data->SCMonitor.recieveFlag=2;

	return true;
}


bool SC_SetConfig::TurnOn()
{
	int counter=0;
   	//------------------------------------Relay Control
	if(m_verbose>1){std::cout<<"Relay Control"<<std::endl;}
	if(m_data->SCMonitor.relayCh1!=m_data->SCMonitor.relayCh1_mon)
	{
		if(m_verbose>3){std::cout << "Relay 1 is " << std::boolalpha << m_data->SCMonitor.relayCh1_mon << " and will be " << std::boolalpha << m_data->SCMonitor.relayCh1  << std::endl;}
		retval = m_data->CB->SetRelay(1,m_data->SCMonitor.relayCh1);
		if(retval!=0 && retval!=1)
		{
			if(m_verbose>3){std::cout << "There was an error (Relay 1) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB01EE01);
		}else
		{
			if(m_verbose>3){std::cout << "Very weird relay behavior! Got bool value other than 0/1!"<<stD::endl;}
		}
	}
	m_data->SCMonitor.relayCh1_mon = m_data->CB->GetRelayState(1);
	m_data->SCMonitor.SumRelays = m_data->SCMonitor.relayCh1_mon;

	//Wait to ensure power is there
	usleep(10000000);
	//------------------------------------HV Control
	if(m_verbose>1){std::cout<<"HV control"<<std::endl;}
	int current_HVstate = 0;
	if(m_data->SCMonitor.HV_state_set!=0)
	{
		//------------------------------------HV Prep
		if(m_verbose>1){std::cout<<"HV Prep"<<std::endl;}
		retval = m_data->CB->SetLV(false);
		if(retval!=0 && retval!=1)
		{
		    if(m_verbose>3){std::cout << " There was an error (Set LV) with retval: " << retval << std::endl;}
		    m_data->SCMonitor.errorcodes.push_back(0xCB02EE01);
		}
		m_data->SCMonitor.HV_mon = current_HVstate;
		
		//Switch HV
		retval = m_data->CB->SetHV_ONOFF(m_data->SCMonitor.HV_state_set);
		if(retval!=0 && retval!=1)
		{
			if(m_verbose>3){std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB03EE01);
		}
		
		//Turn on voltage if it is non zero
		if(m_data->SCMonitor.HV_volt!=0)
		{
			m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
			m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;
		
			if(m_data->SCMonitor.HV_volts!=m_data->SCMonitor.HV_return_mon)
			{
				retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
				if(retval==0)
				{	
					m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
					m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	

					counter = 0;
					while(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
					{
						usleep(10000000);
						m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
						m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
						if(counter>=30){break;}
						counter++;
					}
					if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
					{
						if(m_verbose>1){std::cout << "HV was: " << m_data->SCMonitor.HV_return_mon << std::endl;}
						m_data->SCMonitor.errorcodes.push_back(0xCB03EE04);
					}
					m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
					std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
					outfile << m_data->CB->get_HV_volts;
					outfile.close();
				}else
				{
					if(m_verbose>3){std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;}
					m_data->SCMonitor.errorcodes.push_back(0xCB03EE03);
				}
			}
		}
	}

	//------------------------------------LV Control
	if(m_verbose>1){std::cout<<"LV control"<<std::endl;}
	int current_LVstate = m_data->CB->GetLV_ONOFF();
	if(current_LVstate==0 || current_LVstate==1)
	{
		m_data->SCMonitor.LV_mon = current_LVstate;
	}else
	{
		m_data->SCMonitor.errorcodes.push_back(0xCB04EE00);
	}

	if(m_data->SCMonitor.LV_state_set!=m_data->SCMonitor.LV_mon)
	{
		retval = m_data->CB->SetLV(m_data->SCMonitor.LV_state_set);
		if(retval!=0 && retval!=1)
		{
			if(m_verbose>3){std::cout << " There was an error (Set LV) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB04EE01);
		}
	}

	//------------------------------------Triggerboard Control
	if(m_verbose>1){std::cout<<"Triggerboard Control"<<std::endl;}
	float current_trigger_daq0 = m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref);
	if(m_data->SCMonitor.Trig0_threshold!=current_trigger_daq0)
	{
		retval = m_data->CB->SetTriggerDac0(m_data->SCMonitor.Trig0_threshold, m_data->SCMonitor.TrigVref);
		if(retval!=0)
		{
			if(m_verbose>3){std::cout << " There was an error (DAC0) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE01);
		}
		current_trigger_daq0 = m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref);
		if(fabs(current_trigger_daq0 - m_data->SCMonitor.Trig0_threshold)<0.001)
		{
			m_data->SCMonitor.Trig0_mon = current_trigger_daq0;
		}else
		{
			if(m_verbose>3){std::cout << " There was an error (DAC0) - 0xC0 hasn't been updated!" << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE02);
		}
	}

	float current_trigger_daq1 = m_data->CB->GetTriggerDac1(m_data->SCMonitor.TrigVref);
	if(m_data->SCMonitor.Trig1_threshold!=current_trigger_daq1)
	{
		retval = m_data->CB->SetTriggerDac1(m_data->SCMonitor.Trig1_threshold, m_data->SCMonitor.TrigVref);
		if(retval!=0)
		{
			if(m_verbose>3){std::cout << " There was an error (DAC1) with retval: " << retval << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE03);
		}
		current_trigger_daq1 = m_data->CB->GetTriggerDac1(m_data->SCMonitor.TrigVref);
		if(fabs(current_trigger_daq1 - m_data->SCMonitor.Trig1_threshold)<0.001)
		{
			m_data->SCMonitor.Trig1_mon = current_trigger_daq1;
		}else
		{
			if(m_verbose>3){std::cout << " There was an error (DAC1) - 0xC0 hasn't been updated!" << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE04);
		}
	} 

	m_data->SCMonitor.recieveFlag=2;

	return true;
}



bool SC_SetConfig::Setup(){

	int counter=0;

   	//------------------------------------Relay Control
	if(m_verbose>1){std::cout<<"Relay Control"<<std::endl;}
	if(m_data->SCMonitor.relayCh1!=m_data->SCMonitor.relayCh1_mon)
	{
		//std::cout << "Relay 1 is " << std::boolalpha << m_data->SCMonitor.relayCh1_mon << " and will be " << std::boolalpha << m_data->SCMonitor.relayCh1  << std::endl;
		retval = m_data->CB->SetRelay(1,m_data->SCMonitor.relayCh1);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Relay 1) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB01EE01);
		}
	}

	if(m_data->SCMonitor.relayCh2!=m_data->SCMonitor.relayCh2_mon)
	{
		//std::cout << "Relay 2 is " << std::boolalpha << m_data->SCMonitor.relayCh2_mon << " and will be " << std::boolalpha << m_data->SCMonitor.relayCh2  << std::endl;
		retval = m_data->CB->SetRelay(2,m_data->SCMonitor.relayCh2);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Relay 2) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB01EE02);
		}
	}

	if(m_data->SCMonitor.relayCh3!=m_data->SCMonitor.relayCh3_mon)
	{
		//std::cout << "Relay 3 is " << std::boolalpha << m_data->SCMonitor.relayCh3_mon << " and will be " << std::boolalpha << m_data->SCMonitor.relayCh3  << std::endl;
		retval = m_data->CB->SetRelay(3,m_data->SCMonitor.relayCh3);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Relay 3) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB01EE03);
		}
	}  
	
	m_data->SCMonitor.relayCh1_mon = m_data->CB->GetRelayState(1);
	m_data->SCMonitor.relayCh2_mon = m_data->CB->GetRelayState(2);
	m_data->SCMonitor.relayCh3_mon = m_data->CB->GetRelayState(3);
	if(m_data->SCMonitor.relayCh1_mon==false && m_data->SCMonitor.relayCh2_mon==false && m_data->SCMonitor.relayCh3_mon==false)
	{
		m_data->SCMonitor.SumRelays = false;
		m_data->SCMonitor.recieveFlag=2;
		return true;
	}else if(m_data->SCMonitor.relayCh1_mon==true && m_data->SCMonitor.relayCh2_mon==true && m_data->SCMonitor.relayCh3_mon==true)
    {
        m_data->SCMonitor.SumRelays = true;
    }else
	{
		m_data->SCMonitor.SumRelays = m_data->SCMonitor.relayCh1_mon;
	}
	
  	usleep(10000000);
	//------------------------------------HV Control
	if(m_verbose>1){std::cout<<"HV control"<<std::endl;}
	int temp_HVstate = m_data->CB->GetHV_ONOFF();
	if(temp_HVstate==0 && m_data->SCMonitor.HV_state_set==1)
	{
		//------------------------------------HV Prep
		if(m_verbose>1){std::cout<<"HV Prep"<<std::endl;}
		retval = m_data->CB->SetLV(false);
		if(retval!=0 && retval!=1)
		{
		    //std::cout << " There was an error (Set LV) with retval: " << retval << std::endl;
		    m_data->SCMonitor.errorcodes.push_back(0xCB02EE01);
		}
		
		m_data->SCMonitor.HV_mon = temp_HVstate;
		
		retval = m_data->CB->SetHV_ONOFF(m_data->SCMonitor.HV_state_set);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB03EE01);
		}
		
		m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
		m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;
		if(fabs(m_data->CB->get_HV_volts-m_data->SCMonitor.HV_return_mon)>10)
		{
			//std::cout << "ERROR! " << "File gave " << m_data->CB->get_HV_volts << " Readback gave " << m_data->SCMonitor.HV_return_mon << std::endl;
			//std::cout << "Setting them as the read back value" << std::endl;
			//m_data->SCMonitor.errorcodes.push_back(0xCB03EE02);
			m_data->CB->get_HV_volts = m_data->SCMonitor.HV_return_mon;
		}
		
		if(m_data->SCMonitor.HV_volts!=m_data->SCMonitor.HV_return_mon)
		{
			retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
			if(retval==0)
			{	
				m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
				m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
				counter = 0;
				while(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
				{
					usleep(10000000);
					m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
					m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
					if(counter>=30){break;}
					counter++;
				}
				if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
				{
					if(m_verbose>1){std::cout << "HV was: " << m_data->SCMonitor.HV_return_mon << std::endl;}
					m_data->SCMonitor.errorcodes.push_back(0xCB03EE04);
				}
				m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
				std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
				outfile << m_data->CB->get_HV_volts;
				outfile.close();
			}else
			{
				//std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;
				m_data->SCMonitor.errorcodes.push_back(0xCB03EE03);
			}
		}
	}else if(temp_HVstate==1 && m_data->SCMonitor.HV_state_set==0) 
	{
		if(m_data->SCMonitor.HV_volts!=m_data->SCMonitor.HV_return_mon)
		{
			retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
			if(retval==0)
			{	
				m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
				std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
				outfile << m_data->CB->get_HV_volts;
				outfile.close();
			}else
			{
				//std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;
				m_data->SCMonitor.errorcodes.push_back(0xCB03EE06);
			}
		}
		usleep(5000000);//5s timeout to make sure it can ramp fully down
		m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
		m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
		counter=0;
		while(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
		{
			usleep(10000000);
			m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
			m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
			if(counter>=30){break;}
			counter++;
		}
		if(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
		{
			if(m_verbose>1){std::cout << "HV was: " << m_data->SCMonitor.HV_return_mon << std::endl;}
			m_data->SCMonitor.errorcodes.push_back(0xCB03EE08);
			retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
			if(retval==0)
			{	
			}else
			{
				//std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;
				m_data->SCMonitor.errorcodes.push_back(0xCB03EE09);
			}
		}

        m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
        std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
        outfile << m_data->CB->get_HV_volts;
        outfile.close();
        
		retval = m_data->CB->SetHV_ONOFF(m_data->SCMonitor.HV_state_set);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Set HV) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB03EE07);
		}
	}else if(temp_HVstate==1 && m_data->SCMonitor.HV_state_set==1)
	{
		if(m_data->SCMonitor.HV_volts!=m_data->SCMonitor.HV_return_mon)
		{
			retval = m_data->CB->SetHV_voltage(m_data->SCMonitor.HV_volts,m_data->SCMonitor.HV_return_mon,m_verbose);
			if(retval==0)
			{	
				m_data->CB->get_HV_volts = m_data->SCMonitor.HV_volts;
				std::fstream outfile("./configfiles/SlowControl/LastHV.txt", std::ios_base::out | std::ios_base::trunc);
				outfile << m_data->CB->get_HV_volts;
				outfile.close();
			}else
			{
				//std::cout << " There was an error (HV V set) with retval: " << retval << std::endl;
				m_data->SCMonitor.errorcodes.push_back(0xCB03EE10);
			}
		}

		m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
		m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
		counter = 0;
		while(fabs(m_data->SCMonitor.HV_return_mon-m_data->SCMonitor.HV_volts)>50)
		{
			usleep(10000000);
			m_data->SCMonitor.HV_mon = m_data->CB->GetHV_ONOFF();
			m_data->SCMonitor.HV_return_mon = m_data->CB->ReturnedHvValue;	
			if(counter>=30){break;}
			counter++;
		}
	}else
	{
		//just continue
		//Is already off and set to off
	}
	

	//------------------------------------LV Control
	if(m_verbose>1){std::cout<<"LV control"<<std::endl;}
	int temp_LVstate = m_data->CB->GetLV_ONOFF();
	if(temp_LVstate==0 || temp_LVstate==1)
	{
		m_data->SCMonitor.LV_mon = temp_LVstate;
	}else
	{
		m_data->SCMonitor.errorcodes.push_back(0xCB04EE00);
	}

	if(m_data->SCMonitor.LV_state_set!=m_data->SCMonitor.LV_mon)
	{
		retval = m_data->CB->SetLV(m_data->SCMonitor.LV_state_set);
		if(retval!=0 && retval!=1)
		{
			//std::cout << " There was an error (Set LV) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB04EE01);
		}
	}


	//------------------------------------Triggerboard Control
	if(m_verbose>1){std::cout<<"Triggerboard Control"<<std::endl;}
	float tempval;
	if(m_data->SCMonitor.Trig0_threshold!=m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref))
	{
		retval = m_data->CB->SetTriggerDac0(m_data->SCMonitor.Trig0_threshold, m_data->SCMonitor.TrigVref);
		if(retval!=0)
		{
			//std::cout << " There was an error (DAC0) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE01);
		}
		tempval = m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref);
		if(std::abs(tempval - m_data->SCMonitor.Trig0_threshold)<0.001)
		{
			m_data->SCMonitor.Trig0_mon = tempval;
		}else
		{
			//std::cout << " There was an error (DAC0) - 0xC0 hasn't been updated!" << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE02);
		}
	}

	if(m_data->SCMonitor.Trig1_threshold!=m_data->CB->GetTriggerDac0(m_data->SCMonitor.TrigVref))
	{
		retval = m_data->CB->SetTriggerDac1(m_data->SCMonitor.Trig1_threshold, m_data->SCMonitor.TrigVref);
		if(retval!=0)
		{
			//std::cout << " There was an error (DAC1) with retval: " << retval << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE03);
		}
		tempval = m_data->CB->GetTriggerDac1(m_data->SCMonitor.TrigVref);
		if(std::abs(tempval - m_data->SCMonitor.Trig1_threshold)<0.001)
		{
			m_data->SCMonitor.Trig1_mon = tempval;
		}else
		{
			//std::cout << " There was an error (DAC1) - 0xC0 hasn't been updated!" << std::endl;
			m_data->SCMonitor.errorcodes.push_back(0xCB05EE04);
		}
	} 

	
	m_data->SCMonitor.recieveFlag=2;
	
	return true;	
}
