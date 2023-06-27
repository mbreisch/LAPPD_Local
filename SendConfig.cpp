#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

#include "zmq.hpp"
#include <SlowControlMonitor.h>

using namespace std;


std::map<std::string,std::string> LoadFile()
{
    std::map<std::string,std::string> Settings;

    std::string line;
    std::fstream infile("./configfiles/SlowControl/LocalSettings", std::ios_base::in);
    if(!infile.is_open())
    {
        std::cout<<"File was not found! Please check here: configfiles/Slowcontrol/LocalSettings!"<<std::endl;
    }

    std::string token;
    std::vector<std::string> tokens;
    while(getline(infile, line))
    {
        if(line.empty() || line[0] == '#')
        {
            continue;
        }

        std::stringstream ss(line);
        tokens.clear();
        while(ss >> token) 
        {
            tokens.push_back((std::string)token);
        }
        Settings.insert(std::pair<std::string,std::string>(tokens[0],tokens[1]));
    }

    infile.close();
    return Settings;
}

std::map<std::string,std::string> LoadPreset(std::map<std::string,std::string> Settings,int mode)
{
    if(mode==0)
    {
        //HV settings
        Settings["HV_state_set"] = "0";
        Settings["HV_volts"] = "0";
        //LV settings
        Settings["LV_state_set"] = "0";
        //Relaysettings
        Settings["relayCh1"] = "0";
        Settings["relayCh2"] = "0";
        Settings["relayCh3"] = "0";
    }else if(mode==1)
    {
        //HV settings
        Settings["HV_state_set"] = "0";
        Settings["HV_volts"] = "0";
        //LV settings
        Settings["LV_state_set"] = "0";
        //Relaysettings
        Settings["relayCh1"] = "1";
        Settings["relayCh2"] = "1";
        Settings["relayCh3"] = "1";  
    }else if(mode==2)
    {
        //HV settings
        Settings["HV_state_set"] = "0";
        Settings["HV_volts"] = "0";
        //LV settings
        Settings["LV_state_set"] = "1";
        //Relaysettings
        Settings["relayCh1"] = "1";
        Settings["relayCh2"] = "1";
        Settings["relayCh3"] = "1";  
    }

    return Settings;
}

int getLappdID()
{
    int id;
    std::string line;
    std::fstream infile("./configfiles/SlowControl/SC_Set", std::ios_base::in);
    if(!infile.is_open())
    {
        std::cout<<"File was not found! Please check here: configfiles/Slowcontrol/SC_Set!"<<std::endl;
    }

    std::string token;
    std::vector<std::string> tokens;
    while(getline(infile, line))
    {    
        if(line.empty() || line[0] == '#')
        {
            continue;
        }

        std::stringstream ss(line);
        tokens.clear();
        while (ss >> token) 
        {
            tokens.push_back(token);
        }       
        if(tokens[0]=="LAPPD_ID")
        {
            id = std::stoi(tokens[1]);
            break;
        }
    }
    infile.close();
    return id;
}

void PrintSettings(std::map<std::string,std::string> Settings)
{
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<"These settings will be sent"<<std::endl;
    std::cout<<"LAPPD ID: "<<getLappdID()<<std::endl;
    //HV settings
    std::cout<<"HV_state_set: "<<Settings["HV_state_set"]<<std::endl;
    std::cout<<"HV_volts: "<<Settings["HV_volts"]<<std::endl;
    std::cout<<endl;
    //LV settings
    std::cout<<"LV_state_set: "<<Settings["LV_state_set"]<<std::endl;
    std::cout<<endl;
    //Emergency limits 
    std::cout<<"LIMIT_temperature_low: "<<Settings["LIMIT_temperature_low"]<<std::endl;
    std::cout<<"LIMIT_temperature_high: "<<Settings["LIMIT_temperature_high"]<<std::endl;
    std::cout<<"LIMIT_humidity_low: "<<Settings["LIMIT_humidity_low"]<<std::endl;
    std::cout<<"LIMIT_humidity_high: "<<Settings["LIMIT_humidity_high"]<<std::endl;
    std::cout<<endl;
    std::cout<<"LIMIT_Thermistor_temperature_low): "<<Settings["LIMIT_Thermistor_temperature_low"]<<std::endl;
    std::cout<<"LIMIT_Thermistor_temperature_high: "<<Settings["LIMIT_Thermistor_temperature_high"]<<std::endl;
    std::cout<<endl;
    std::cout<<"LIMIT_saltbridge_low: "<<Settings["LIMIT_saltbridge_low"]<<std::endl;
    std::cout<<"LIMIT_saltbridge_high: "<<Settings["LIMIT_saltbridge_high"]<<std::endl;
    std::cout<<endl;
    //Trigger
    std::cout<<"Trig0_threshold: "<<Settings["Trig0_threshold"]<<std::endl;
    std::cout<<"Trig1_threshold: "<<Settings["Trig1_threshold"]<<std::endl;
    std::cout<<"TrigVref: "<<Settings["TrigVref"]<<std::endl;
    std::cout<<endl;
    //Relays
    std::cout<<"relayCh1: "<<Settings["relayCh1"]<<std::endl;
    std::cout<<"relayCh2: "<<Settings["relayCh2"]<<std::endl;
    std::cout<<"relayCh3: "<<Settings["relayCh3"]<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<<std::endl;
}

int SetSettings(std::map<std::string,std::string> Settings)
{
    zmq::context_t context;
    zmq::socket_t sock(context, ZMQ_DEALER);
    sock.connect("tcp://127.0.0.1:4444");

    SlowControlMonitor data;

    //HV settings
    if(std::stoi(Settings["HV_state_set"])==0)
    {
        data.HV_state_set = false;
    }else
    {
        data.HV_state_set = true;
    }
    data.HV_volts = std::stof(Settings["HV_volts"]);
    //LV settings
    if(std::stoi(Settings["LV_state_set"])==0)
    {
        data.LV_state_set = false;
    }else
    {
        data.LV_state_set = true;
    }
    //Limits
    data.LIMIT_temperature_low = std::stof(Settings["LIMIT_temperature_low"]);
    data.LIMIT_temperature_high = std::stof(Settings["LIMIT_temperature_high"]);
    data.LIMIT_humidity_low = std::stof(Settings["LIMIT_humidity_low"]);
    data.LIMIT_humidity_high = std::stof(Settings["LIMIT_humidity_high"]);
    data.LIMIT_Thermistor_temperature_low = std::stof(Settings["LIMIT_Thermistor_temperature_low"]);
    data.LIMIT_Thermistor_temperature_high = std::stof(Settings["LIMIT_Thermistor_temperature_high"]);
    data.LIMIT_saltbridge_low = std::stof(Settings["LIMIT_saltbridge_low"]);
    data.LIMIT_saltbridge_high = std::stof(Settings["LIMIT_saltbridge_high"]);
    //Trigger
    data.Trig0_threshold = std::stof(Settings["Trig0_threshold"]);
    data.Trig1_threshold = std::stof(Settings["Trig1_threshold"]);
    data.TrigVref = std::stof(Settings["TrigVref"]);
    //Relaysettings
    if(std::stoi(Settings["relayCh1"])==0)
    {
        data.relayCh1 = false;
    }else
    {
        data.relayCh1 = true;
    }
    if(std::stoi(Settings["relayCh2"])==0)
    {
        data.relayCh2 = false;
    }else
    {
        data.relayCh2 = true;
    }
    if(std::stoi(Settings["relayCh3"])==0)
    {
        data.relayCh3 = false;
    }else
    {
        data.relayCh3 = true;
    }

    PrintSettings(Settings);
    std::cout<<"Do you want to send the above settings? ";
    bool send=false;
    std::cin>>send;

    if(!send)
    {
        return 0;
    }else
    {
        data.Send_Config(&sock);  
        return 1;
    }
}


int main()
{
    int choice = -1;
    int ret = -1;
    std::string yn;
    std::map<std::string,std::string> Settings;
    while(true)
    {
        std::cout<<"NOTICE!!!! This now works via a configfile in configfiles/Slowcontrol/LocalSettings"<<std::endl;
        std::cout<<"Did you read this! (y/n) ";
        std::cin >> yn;
        if(yn=="y"){break;}
    }

    while(true)
    {
        std::cout<<std::endl;
        std::cout<<"What do you want to do?"<<std::endl;
        std::cout<<"(1) Load and set settings"<<std::endl;
        std::cout<<"(2) Check current settings"<<std::endl;
        std::cout<<"(3) Turn everything off"<<std::endl;
        std::cout<<"(4) Passive modes"<<std::endl;
        std::cout<<"(5) Exit"<<std::endl;
        std::cout<<"Enter: ";
        std::cin >> choice;

        if(choice==1)
        {   
            Settings = LoadFile();
            ret = SetSettings(Settings);
            if(ret==0)
            {
                std::cout<<"Exiting without setting"<<std::endl;
            }else
            {
                std::cout<<"Exiting with new Settings"<<std::endl;
            }
        }else if(choice==2)
        {
            Settings = LoadFile();
            PrintSettings(Settings);
        }else if(choice==3)
        {
            Settings = LoadFile();
            Settings = LoadPreset(Settings,0);
            ret = SetSettings(Settings);
            if(ret==0)
            {
                std::cout<<"Exiting without setting"<<std::endl;
            }else
            {
                std::cout<<"Exiting with new Settings"<<std::endl;
            }
        }else if(choice==4)
        {
            int choice2 = -1;
            while(true)
            {
                std::cout<<endl;
                std::cout<<"What passive mode do you want to load?"<<std::endl;
                std::cout<<"(1) Relays only"<<std::endl;
                std::cout<<"(2) LV on"<<std::endl;
                std::cout<<"(3) Exit"<<std::endl;

                std::cin >> choice2;

                if(choice2==1)
                {
                    Settings = LoadFile();
                    Settings = LoadPreset(Settings,1);
                    ret = SetSettings(Settings);
                    if(ret==0)
                    {
                        std::cout<<"Exiting without setting"<<std::endl;
                    }else
                    {
                        std::cout<<"Exiting with new Settings"<<std::endl;
                    }
                }else if(choice2==2)
                {
                    Settings = LoadFile();
                    Settings = LoadPreset(Settings,2);
                    ret = SetSettings(Settings);
                    if(ret==0)
                    {
                        std::cout<<"Exiting without setting"<<std::endl;
                    }else
                    {
                        std::cout<<"Exiting with new Settings"<<std::endl;
                    }
                }else if(choice2==3)
                {
                    break;
                }else
                {
                    std::cout<<"Please enter a valid entry!"<<std::endl;
                }
            }
        }else if(choice==5)
        {
            std::cout<<"Exiting"<<std::endl;
            break;
        }else
        {
            std::cout<<"Please enter a valid entry!"<<std::endl;
        }
    }
}