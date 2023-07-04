#include "ACC_SetupBoards.h"

ACC_SetupBoards::ACC_SetupBoards():Tool(){}


bool ACC_SetupBoards::Initialise(std::string configfile, DataModel &data){
	
	m_data= &data;
	m_log= m_data->Log;
	
	// allow overrides from local config file
	localconfigfile=configfile;   // note for reinit
	if(configfile!="")  m_variables.Initialise(configfile);
	
	if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
	
	//m_variables.Print();
	
	if(m_data->acc==nullptr) m_data->acc = new ACC();
	
	TimeoutResetCount = 300;
	PPSWaitMultiplier = 10;
	m_variables.Get("PPSWaitMultiplier",PPSWaitMultiplier);
	
	return true;
}


bool ACC_SetupBoards::Execute()
{
	bool StartReset = false;
	for(std::map<int, int>::iterator it=m_data->TCS.Timeoutcounter.begin(); it!=m_data->TCS.Timeoutcounter.end(); ++it)
	{
        	if(it->second>TimeoutResetCount)
        	{
			std::cout << "Timeout of " << it->first << " with " << it->second << " against " << TimeoutResetCount << std::endl;
			StartReset = true;
		}
	}

	if(StartReset==true)
	{
        m_data->config.receiveFlag = 1; //Re-init the Setup part uf the tool

        //
        //  Here will be a new print later
        //

		for(std::map<int, int>::iterator it=m_data->TCS.Timeoutcounter.begin(); it!=m_data->TCS.Timeoutcounter.end(); ++it)
		{
			m_data->TCS.Timeoutcounter.at(it->first) = 0; //Reset the timeout counter
		}
		if(m_data->config.ResetSwitchACC == 1)
		{
			m_data->acc->resetACC();
		}
		if(m_data->config.ResetSwitchACDC == 1)
		{
			m_data->acc->resetACDC();
		}
    }

	bool setupret = false;
	if(m_data->config.receiveFlag==1)
	{
		if(m_data->config.RunControl==0 || m_data->config.RunControl==1)
		{
			m_data->data.errorcodes.clear();
			m_data->data.ReceiveData.clear();
			m_data->data.BoardIndex.clear();
			m_data->data.AccInfoFrame.clear();
			m_data->data.RawWaveform.clear();
			m_data->config.RunControl=-1;

			m_data->acc->resetACC();
			m_data->acc->resetACDC();
		}
		setupret = Setup();
		return setupret;
	}

	return true;
}


bool ACC_SetupBoards::Finalise()
{
	delete m_data->acc;
	m_data->acc = nullptr;
	return true;
}


bool ACC_SetupBoards::Setup()
{
	bool ret=false;
	
	//Set timeout value
	int timeout;
	m_variables.Get("timeout",timeout);
	m_data->acc->setTimeoutInMs(timeout);
	
	//polarity
	m_data->acc->setSign(m_data->config.ACC_Sign, 2);
	m_data->acc->setSign(m_data->config.ACDC_Sign, 3);
	m_data->acc->setSign(m_data->config.SELF_Sign, 4);
	
	//self trigger options
	m_data->acc->setEnableCoin(m_data->config.SELF_Enable_Coincidence);
	
	unsigned int coinNum;
	stringstream ss;
	ss << std::hex << m_data->config.SELF_Coincidence_Number;
	coinNum = std::stoul(ss.str(),nullptr,16);
	m_data->acc->setNumChCoin(coinNum);
	
	unsigned int threshold;
	stringstream ss2;
	ss2 << std::hex << m_data->config.SELF_threshold;
	threshold = std::stoul(ss2.str(),nullptr,16);
	m_data->acc->setThreshold(threshold);
	
	//psec masks combine
	std::vector<int> PsecChipMask = {m_data->config.PSEC_Chip_Mask_0,m_data->config.PSEC_Chip_Mask_1,m_data->config.PSEC_Chip_Mask_2,m_data->config.PSEC_Chip_Mask_3,m_data->config.PSEC_Chip_Mask_4};
	std::vector<unsigned int> VecPsecChannelMask = {m_data->config.PSEC_Channel_Mask_0,m_data->config.PSEC_Channel_Mask_1,m_data->config.PSEC_Channel_Mask_2,m_data->config.PSEC_Channel_Mask_3,m_data->config.PSEC_Channel_Mask_4};
	m_data->acc->setPsecChipMask(PsecChipMask);
	m_data->acc->setPsecChannelMask(VecPsecChannelMask);
	
	//validation window
	unsigned int validationStart;
	stringstream ss31;
	ss31 << std::hex << (int)m_data->config.Validation_Start/25;
	validationStart = std::stoul(ss31.str(),nullptr,16);
	m_data->acc->setValidationStart(validationStart);
	
	unsigned int validationWindow;
	stringstream ss32;
	ss32 << std::hex << (int)m_data->config.Validation_Window/25;
	validationWindow = std::stoul(ss32.str(),nullptr,16);
	m_data->acc->setValidationWindow(validationWindow);
	
	
	//pedestal set
	////set value
	unsigned int pedestal;
	stringstream ss4;
	ss4 << std::hex << m_data->config.Pedestal_channel;
	pedestal = std::stoul(ss4.str(),nullptr,16);
	////set mask
	m_data->acc->setPedestals(m_data->config.ACDC_mask,m_data->config.Pedestal_channel_mask,pedestal);
	
	
	//pps settings
	unsigned int ppsratio;
	stringstream ss5;
	ss5 << std::hex << m_data->config.PPSRatio;
	ppsratio = std::stoul(ss5.str(),nullptr,16);
	m_data->acc->setPPSRatio(ppsratio);
	
	//SetMaxTimeoutValue
	TimeoutResetCount = (PPSWaitMultiplier*m_data->config.PPSRatio)/(m_data->TCS.Timeoutcounter.size()*(timeout/1000.0));
	if(m_verbose>2){std::cout << "Created new timeout value based on " << m_data->config.PPSRatio << " with " << TimeoutResetCount << std::endl;}

	m_data->acc->setPPSBeamMultiplexer(m_data->config.PPSBeamMultiplexer);

    //SMA debug settigns
    m_data->acc->setSMA_Debug(m_data->config.SMA_PPS,m_data->config.SMA_Beamgate);
	
	int retval;
	retval = m_data->acc->initializeForDataReadout(m_data->config.triggermode, m_data->config.ACDC_mask, m_data->config.Calibration_Mode);
	if(retval != 0)
	{
		m_data->data.errorcodes.push_back(0xDA01EE01);
		ret = false;
	}else
	{
		ret = true;
		//std::cout << "Initialization successfull!" << std::endl;
	}
	
	m_data->config.receiveFlag = 2;
	m_data->acc->emptyUsbLine();
	m_data->acc->dumpData(0xFF);
	
	vector<unsigned int> tmpERR = m_data->acc->returnErrors();
	if(tmpERR.size()==1 && tmpERR[0]==0x00000000)
	{
		m_data->data.errorcodes.insert(std::end(m_data->data.errorcodes), std::begin(tmpERR), std::end(tmpERR));
	}
	m_data->acc->clearErrors();
	tmpERR.clear();
	
	return ret;
}
