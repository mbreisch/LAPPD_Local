#include "ACC_DataRead.h"

ACC_DataRead::ACC_DataRead():Tool(){}


bool ACC_DataRead::Initialise(std::string configfile, DataModel &data){
	
	m_data= &data;
	m_log= m_data->Log;
	
	// allow overrides from local config file
	localconfigfile=configfile;
	if(configfile!="") m_variables.Initialise(configfile);
	
	//m_variables.Print();
	
	if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

	//Load LAPPD_ID infos
	LAPPD_ID = -1;
	m_variables.Get("LAPPD_ID",LAPPD_ID);
	m_variables.Get("Port_0",Port_0);
	m_variables.Get("Port_1",Port_1);
	LAPPD_on_ACC = {Port_0,Port_1};

	PreviousBuffer.insert(pair<int, unsigned short>(Port_0, 0));
	PreviousBuffer.insert(pair<int, unsigned short>(Port_1, 0));

	m_data->TCS.Timeoutcounter.insert(pair<int, int>(LAPPD_ID, 0));
	
	return true;
}

bool ACC_DataRead::Execute()
{
    try
    {
        m_data->data.LAPPD_ID = LAPPD_ID;
        if(m_data->config.triggermode==1)
        {
            std::cout << "Software trigger firing ---->" << std::endl;
            m_data->acc->softwareTrigger();
        }
        m_data->data.readRetval = m_data->acc->listenForAcdcData(m_data->config.triggermode,LAPPD_on_ACC);
    }catch(std::exception& e)
    {
        std::cerr<<"ACC_DataRead::Execute caught exception on read "<<e.what()<<std::endl;
    }

    try
    {
        if(m_data->data.readRetval != 0)
        {
            if(m_data->data.readRetval != 404)
            {
                if(m_verbose>1){printf("not 404 but %i\n",m_data->data.readRetval);}
                if(m_data->data.readRetval==405){m_data->vars.Set("StopLoop",1);}
                m_data->TCS.Timeoutcounter.at(LAPPD_ID) = 0;
                m_data->data.FailedReadCounter = m_data->data.FailedReadCounter + 1;
                m_data->data.errorcodes.push_back(0xAD02EE01);
                unsigned int dumpIndex = 0x00 | (1<<LAPPD_on_ACC[0]) | (1<<LAPPD_on_ACC[1]);
                m_data->acc->dumpData(dumpIndex);
                m_data->acc->emptyUsbLine();
                m_data->data.AccInfoFrame = m_data->acc->returnACCIF();
            }else
            {
                if(m_verbose>2){printf("404\n");}
                /*
                vector<unsigned short> TMP_ACC = m_data->acc->returnACCIF();
                if(PreviousBuffer[Port_0]==0 && PreviousBuffer[Port_1]==0)
                {
                    PreviousBuffer[Port_0] = TMP_ACC[16+Port_0];
                    PreviousBuffer[Port_1] = TMP_ACC[16+Port_1];
                }else
                {
                    if(PreviousBuffer[Port_0]==TMP_ACC[16+Port_0] && PreviousBuffer[Port_1]==TMP_ACC[16+Port_1])
                    {
                        m_data->data.errorcodes.push_back(0xAA02EE11);
                        unsigned int dumpIndex = 0x00 | (1<<LAPPD_on_ACC[0]) | (1<<LAPPD_on_ACC[1]);
                        m_data->acc->dumpData(dumpIndex);
                    }else
                    {
                        PreviousBuffer[Port_0] = TMP_ACC[16+Port_0];
                        PreviousBuffer[Port_1] = TMP_ACC[16+Port_1];		
                    }
                }
                TMP_ACC.clear();
                */
                m_data->TCS.Timeoutcounter.at(LAPPD_ID) = m_data->TCS.Timeoutcounter.at(LAPPD_ID) + 1;
            }
            m_data->data.ReceiveData.clear();
            m_data->acc->clearData();
        }else{
            if(m_verbose>3){printf("good data\n");}
            m_data->TCS.Timeoutcounter.at(LAPPD_ID) = 0;
            m_data->data.AccInfoFrame = m_data->acc->returnACCIF();
            m_data->data.ReceiveData = m_data->acc->returnRaw();
            m_data->data.BoardIndex = m_data->acc->returnBoardIndices();
            m_data->data.RawWaveform = m_data->data.ReceiveData;
            m_data->acc->clearData();
            if(m_verbose>3){m_data->data.Print();}
        }                
    }catch(std::exception& e)
    {
        std::cerr<<"ACC_DataRead::Execute caught exception on read handling "<<e.what()<<std::endl;
        return false;
    }      
                          
    //Get Timestamp
    unsigned long long timeSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_data->data.Timestamp = to_string(timeSinceEpoch);

    try
    {
        //Get errors
        vector<unsigned int> tmpERR = m_data->acc->returnErrors();
        if(tmpERR.size()!=0)
        {
            m_data->data.errorcodes.insert(std::end(m_data->data.errorcodes), std::begin(tmpERR), std::end(tmpERR));
        }
        m_data->acc->clearErrors();
        tmpERR.clear();
    }catch(std::exception& e)
    {
        std::cerr<<"ACC_DataRead::Execute caught exception errorcode grab "<<e.what()<<std::endl;
        return false;
    }

    return true;
}


bool ACC_DataRead::Finalise(){

  return true;
}
