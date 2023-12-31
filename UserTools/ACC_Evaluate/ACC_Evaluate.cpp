#include "ACC_Evaluate.h"

ACC_Evaluate::ACC_Evaluate():Tool(){}


bool ACC_Evaluate::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

    seperator = "--------------------"; 

	m_variables.Get("Port_0",Port_0);
	m_variables.Get("Port_1",Port_1);
	LAPPD_on_ACC = {Port_0,Port_1};

    logfile.open("./RunLog",ios_base::app);

    loglength = 50000;

    return true;
}


bool ACC_Evaluate::Execute()
{
    std::vector<unsigned short> ACC_IF = m_data->data.AccInfoFrame;
    if(ACC_IF.size()==0){return true;}

    if(m_verbose>4){Print_ACC_IF(ACC_IF);}
    if(m_verbose>1){Print_Buffer_Debug(ACC_IF);}

    return true;
}


bool ACC_Evaluate::Finalise()
{
    logfile.close();
    return true;
}


void ACC_Evaluate::Print_ACC_IF(std::vector<unsigned short> accif)
{
    printf("%s",seperator);
    printf("Frame length was %li words\n",accif.size());
    printf("Firmware version was %i from %i/%i\n",accif.at(accif.at(2)),accif.at(accif.at(3)),accif.at(accif.at(4)));
    printf("External clock bit is %i and PLL lock is %i with %i failures\n",(0x1 & accif.at(12)),(0x2 & accif.at(12)),accif.at(10));
}


void ACC_Evaluate::Print_Buffer_Debug(std::vector<unsigned short> accif)
{
    if(accif.size()==0){return;}
    unsigned short word14 = accif.at(14);

    int bit0 = word14 & (1<<Port_0);
    int bit1 = word14 & (1<<Port_1);

    int buffer0 = accif.at(16+Port_0);
    int buffer1 = accif.at(16+Port_1);

    vector<unsigned short> TmpVector = m_data->data.RawWaveform;
    if(TmpVector.size()==0){return;}

    unsigned short pps_c1;
    unsigned short pps_c2;
    try
    {
        pps_c1 = TmpVector.at(9);
        pps_c2 = TmpVector.at(8);  
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error at PPS call for counter: " << e.what() << '\n';
    }
    
    stringstream ss_cPPS;
    ss_cPPS << std::setfill('0') << std::setw(4) << std::hex << pps_c2;
    ss_cPPS << std::setfill('0') << std::setw(4) << std::hex << pps_c1;
    unsigned long long PPS_Counter = std::stoull(ss_cPPS.str(),nullptr,16);

    printf(">>>> %s: %i - %i | %i - %i with ID: %llu\n",m_data->data.Timestamp.c_str(),bit0,buffer0,bit1,buffer1,PPS_Counter);

    if(GetFileLength()<loglength)
    {
        std::string time = m_data->data.Timestamp.c_str();
        logfile << time <<": "<<bit0<<" - "<<buffer0<<" | "<<bit1<<" - "<<buffer1<<" with: "<<PPS_Counter<<endl;
    } 
}


int ACC_Evaluate::GetFileLength()
{
    std::string filename = "./RunLog"; 

    std::ifstream file(filename);
    if(!file) 
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    int lineCount = 0;
    std::string line;
    while(std::getline(file, line)) 
    {
        lineCount++;
    }
    
    file.close();
    return lineCount;
}