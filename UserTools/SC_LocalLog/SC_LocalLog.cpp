#include "SC_LocalLog.h"

SC_LocalLog::SC_LocalLog():Tool(){}


bool SC_LocalLog::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    system("mkdir -p LocalLogs");

    logrun = 0;

    return true;
}


bool SC_LocalLog::Execute()
{
    if(m_data->SCMonitor.timeSinceEpochMilliseconds=="")
    {
        return true;
    }

    if(logrun>=10)
    {
        logrun = 0;
    }

    // get filelocation
    std::string filelocation = "./LocalLogs/LocalLog_";
    filelocation += to_string(logrun);
    filelocation += ".txt";
 
    // epoch time in ms
    long long epoch_time_ms = std::stoll(m_data->SCMonitor.timeSinceEpochMilliseconds);
    // convert to seconds
    time_t epoch_time_sec = epoch_time_ms / 1000;
    // convert to local time
    struct tm* local_time = localtime(&epoch_time_sec);
    writeLogFile(asctime(local_time),filelocation);
    writeLogFile(asctime(local_time),"./LocalLogs/LastLog.txt");

    logrun++;

    return true;
}


bool SC_LocalLog::Finalise()
{

    return true;
}


void SC_LocalLog::writeLogFile(string local_time, string savelocation)
{
    std::fstream logfile(savelocation, std::ios_base::out | std::ios_base::trunc);
    logfile << "Time of log: " << local_time << std::endl;
    logfile << "Now printing all available values available. All values read after this tool will be from the last read cycle!" << std::endl;
    logfile << "System timestamp " <<  m_data->SCMonitor.timeSinceEpochMilliseconds << " ms since epoch" << std::endl;
    logfile << "LAPPD ID is " << m_data->SCMonitor.LAPPD_ID << std::endl;
	logfile << "Timestamp ms since epoch = " << m_data->SCMonitor.timeSinceEpochMilliseconds << std::endl;
	logfile << "humidity = " << m_data->SCMonitor.humidity_mon << std::endl;
	logfile << "temperature = " << m_data->SCMonitor.temperature_mon << std::endl;
	logfile << "thermistor = " << m_data->SCMonitor.temperature_thermistor << std::endl;
	logfile << "HV state should be " << std::boolalpha << m_data->SCMonitor.HV_state_set << " and is " << std::boolalpha << m_data->SCMonitor.HV_mon << " and is set to " << m_data->SCMonitor.HV_volts << " V" << std::endl;
	logfile << "Returned HV is " << m_data->SCMonitor.HV_return_mon << "V" << std::endl;
	logfile << "LV state should be " << std::boolalpha << m_data->SCMonitor.LV_state_set << " and is " << std::boolalpha << m_data->SCMonitor.LV_mon << std::endl;
	logfile << "LV voltages are V(3.3)= " << m_data->SCMonitor.v33 << "V, V(3.1)= " << m_data->SCMonitor.v25 << "V, V(1.8)= " << m_data->SCMonitor.v12 << "V" << std::endl;	
	logfile << "Temperature warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_temperature << std::endl;
	logfile << "Humidity warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_humidity << std::endl;
	logfile << "Temperature 2 warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_temperature_Thermistor << std::endl;
	logfile << "Saltbridge warning flag is " << std::boolalpha << m_data->SCMonitor.FLAG_saltbridge << std::endl;
	logfile << "Relay 1 is " << std::boolalpha << m_data->SCMonitor.relayCh1_mon << std::endl;
	logfile << "Relay 2 is " << std::boolalpha << m_data->SCMonitor.relayCh2_mon << std::endl;
	logfile << "Relay 3 is " << std::boolalpha << m_data->SCMonitor.relayCh3_mon << std::endl;
	logfile << "Threshold for DAC 0 is " << m_data->SCMonitor.Trig0_mon << " V" << std::endl;
	logfile << "Threshold for DAC 1 is " << m_data->SCMonitor.Trig1_mon << " V" << std::endl;
	logfile << "Photodiode return is " << m_data->SCMonitor.light << std::endl;
	logfile << "Saltbridge return is " << m_data->SCMonitor.saltbridge << std::endl;
	if (m_data->SCMonitor.errorcodes.size()==0)
    {
        logfile << "No errorcodes found all good " << "0x00000000" << std::endl;
    }
    else if(m_data->SCMonitor.errorcodes.size()==1 && m_data->SCMonitor.errorcodes[0]==0x00000000)
	{
        logfile << "No errorcodes found all good " << "0x" << std::setfill('0') << std::setw(8) << std::hex << m_data->SCMonitor.errorcodes[0] << std::dec << std::endl;
		//printf("No errorcodes found all good: 0x%08x\n", m_data->SCMonitor.errorcodes[0]);
	}else
	{
        logfile << "Errorcodes found, amount = " << m_data->SCMonitor.errorcodes.size() << std::endl;
		//printf("Errorcodes found: %li\n", m_data->SCMonitor.errorcodes.size());
        if(m_data->SCMonitor.errorcodes.size()<100)
        {            
            for(unsigned int k: m_data->SCMonitor.errorcodes)
            {
                logfile << "0x" << std::setfill('0') << std::setw(8) << std::hex << k << std::dec << std::endl;
                //printf("0x%08x\n",k);	
            }
        }
	}
    logfile << "-------------" << std::endl; 
    logfile.close();
}
