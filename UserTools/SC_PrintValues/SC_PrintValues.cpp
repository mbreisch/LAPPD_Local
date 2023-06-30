#include "SC_PrintValues.h"

SC_PrintValues::SC_PrintValues():Tool(){}


bool SC_PrintValues::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;

    if(!m_variables.Get("Period",time_sec)) time_sec=1;

    period=boost::posix_time::seconds(time_sec);
    last=boost::posix_time::second_clock::local_time();

    return true;
}


bool SC_PrintValues::Execute()
{
    boost::posix_time::ptime current(boost::posix_time::second_clock::local_time());
    boost::posix_time::time_duration lapse(period - (current - last)); 

    if(lapse.is_negative())
    {
        PrintMonitoring();
        last=boost::posix_time::second_clock::local_time();
    }

    m_data->SCMonitor.errorcodes.clear();

    return true;
}


bool SC_PrintValues::Finalise()
{
    return true;
}


std::string GetString(int amount, std::string symbol)
{
    std::string str;
    for(int i=0;i<amount;i++){str+=symbol;}

    return str;
}

void SC_PrintValues::PrintMonitoring()
{
    int precision = 100;
    std::string empty; for(int i=0;i<precision;i++){empty+=" ";}
    std::string color;
    std::string state;

    std::cout << "\033[2J\033[1;1H";
    std::cout << "------------------------ SLOW CONTROLS ------------------------" << std::endl;
    std::cout << "This is your monitoring screen for LAPPD-ID " << m_data->SCMonitor.LAPPD_ID << std::endl;
    std::cout << std::endl;

    // Update Temperature
    if(m_data->SCMonitor.temperature_mon >= m_data->SCMonitor.LIMIT_temperature_high){color=red;}
    else if(m_data->SCMonitor.temperature_mon >= m_data->SCMonitor.LIMIT_temperature_low && m_data->SCMonitor.temperature_mon < m_data->SCMonitor.LIMIT_temperature_high){color=yellow;}
    else if(m_data->SCMonitor.temperature_mon < m_data->SCMonitor.LIMIT_temperature_low){color=green;}
    else{std::cout << "Help" << std::endl;}
    std::cout << "Temperature: \t|" << color << GetString((int)m_data->SCMonitor.temperature_mon,"\u2588") << GetString(precision-(int)m_data->SCMonitor.temperature_mon," ");
    std::cout << cnormal << "| " << m_data->SCMonitor.temperature_mon << " deg C" << std::endl;
    std::cout << std::endl;

    // Update Thermistor
    if(m_data->SCMonitor.temperature_thermistor <= m_data->SCMonitor.LIMIT_Thermistor_temperature_high ){color=red;}
    else if(m_data->SCMonitor.temperature_thermistor <= m_data->SCMonitor.LIMIT_Thermistor_temperature_low  && m_data->SCMonitor.temperature_thermistor > m_data->SCMonitor.LIMIT_Thermistor_temperature_high ){color=yellow;}
    else if(m_data->SCMonitor.temperature_thermistor>m_data->SCMonitor.LIMIT_Thermistor_temperature_low){color=green;}
    else{std::cout << "Help" << std::endl;}
    float temp = log((m_data->SCMonitor.temperature_thermistor/1000)/29.4)/(-0.0437);
    std::cout << "Thermistor: \t|" << color << GetString((int)temp,"\u2588") << GetString(precision-(int)temp," ");
    std::cout << cnormal << "| " << temp << " deg C" << std::endl;
    std::cout << std::endl;

    // Update Humidity
    float tool_humidity_limit=0;
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

    if(m_data->SCMonitor.humidity_mon >= tool_humidity_limit){color=red;}
    else if(m_data->SCMonitor.humidity_mon >= m_data->SCMonitor.LIMIT_humidity_low && m_data->SCMonitor.humidity_mon < tool_humidity_limit){color=yellow;}
    else if(m_data->SCMonitor.humidity_mon<m_data->SCMonitor.LIMIT_humidity_low){color=green;}
    else{std::cout << "Help" << std::endl;}
    std::cout << "Humidity: \t\t|" << color << GetString((int)m_data->SCMonitor.humidity_mon,"\u2588");
    std::cout << GetString(precision-(int)m_data->SCMonitor.humidity_mon," ") << cnormal << "| " << m_data->SCMonitor.humidity_mon << " %" << std::endl << std::endl;

    // Update LV
    if(m_data->SCMonitor.LV_mon==0){color=red;state="OFF";}else if(m_data->SCMonitor.LV_mon==1){color=green;state="ON";}else{color=yellow;state="NAN";}
    std::cout << "Low Voltage is \t" << color << state << cnormal << "| " << m_data->SCMonitor.v33 << "|3.3V\t " << m_data->SCMonitor.v25 << "|3.1V\t " << m_data->SCMonitor.v12 << "|1.8V" << std::endl;
    std::cout << std::endl;

    // Update HV
    if(m_data->SCMonitor.HV_mon==0){color=red;state="OFF";}else if(m_data->SCMonitor.HV_mon==1){color=green;state="ON";}else{color=yellow;state="NAN";}
    std::cout << "High Voltage is \t" << color << state << cnormal << "| " << m_data->SCMonitor.HV_return_mon << "|" << m_data->SCMonitor.HV_volts << std::endl;
    std::cout << std::endl;

    // Saltbridge & light
    if(m_data->SCMonitor.saltbridge>m_data->SCMonitor.LIMIT_saltbridge_low){color=green;}
    else if(m_data->SCMonitor.saltbridge<=m_data->SCMonitor.LIMIT_saltbridge_low && m_data->SCMonitor.saltbridge>m_data->SCMonitor.LIMIT_saltbridge_high){color=yellow;}
    else if(m_data->SCMonitor.saltbridge<=m_data->SCMonitor.LIMIT_saltbridge_high){color=red;}
    std::cout << "Saltbridge value is \t" << color << m_data->SCMonitor.saltbridge << cnormal << " | \t Photodiode reads " << m_data->SCMonitor.light << std::endl << std::endl;

    // Update Relays
    std::string c1,c2,c3,s1,s2,s3;
    if(m_data->SCMonitor.relayCh1_mon){c1=green;s1="OPEN";}else{c1=red;s1="CLOSED";}
    if(m_data->SCMonitor.relayCh2_mon){c2=green;s2="OPEN";}else{c2=red;s2="CLOSED";}
    if(m_data->SCMonitor.relayCh3_mon){c3=green;s3="OPEN";}else{c3=red;s3="CLOSED";}

    std::cout << "Relay 1 is " << c1 << s1 << cnormal << " | " << "Relay 2 is " << c2 << s2 << cnormal << " | ";
    std::cout << "Relay 3 is " << c3 << s3 << cnormal << " | " << std::endl << std::endl;

    // Update Errorcodes
    std::cout << "Errorcodes are:" << std::endl;
    for(unsigned int err: m_data->SCMonitor.errorcodes)
    {
        std::cout << "0x" << std::hex << err << std::dec << std::endl;
    }
    
    std::cout << std::endl;
    std::cout <<  "To Quit use Crtl+C, but don't forget to turn down the voltages first" << std::endl;
}