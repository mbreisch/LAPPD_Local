#include "ACC_SaveData.h"

ACC_SaveData::ACC_SaveData():Tool(){}


bool ACC_SaveData::Initialise(std::string configfile, DataModel &data)
{
    if(configfile!="")  m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data= &data;
    m_log= m_data->Log;

    if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
    if(!m_variables.Get("SaveMode",SaveMode)) SaveMode=1;

    if(!m_variables.Get("MaxNumberEvents",MaxNumberEvents)) MaxNumberEvents=0;
    if(!m_variables.Get("EventsPerFile",EventsPerFile)) EventsPerFile=1;

    if(!m_variables.Get("Path",Path)) Path="./Results";
    Path+= getTime();

    if(!m_variables.Get("StoreLabel",StoreLabel)) StoreLabel="LAPPD_INV_";

    FileCounter = 0;
    channel_count = 0;
    starttime = getTime();

    if(SaveMode==2)
	{
		// Make the ANNIEEvent Store if it doesn't exist
		int recoeventexists = m_data->Stores.count("LAPPDStore");
		if(recoeventexists==0)
		{
			m_data->Stores["LAPPDStore"] = new BoostStore(false,2);
		}
	}

    return true;
}


bool ACC_SaveData::Execute()
{
    bool ret = true;

    if(m_data->TCS.EventCounter>=MaxNumberEvents)
    {
        m_variables.Set("StopLoop",1);
        return true;
    }

    if(SaveMode==0)
    {
        m_data->TCS.EventCounter++;
    }else if(SaveMode==1)
    {
        ret = SaveASCII();
    }else if(SaveMode==2)
    {
        ret = SaveStore();
    }else if(SaveMode==3)
    {
        ret = SaveRAW();
    }else
    {
        ret = false;
    }

    //Cleanup	
	m_data->data.AccInfoFrame.clear();
	m_data->data.BoardIndex.clear();
	m_data->data.ReceiveData.clear();
    m_data->data.RawWaveform.clear();

    return ret;
}


bool ACC_SaveData::Finalise()
{
	if(SaveMode==2)
	{
		m_data->Stores["LAPPDStore"]->Close();
		delete m_data->Stores["LAPPDStore"];
		m_data->Stores["LAPPDStore"] = 0;
    }

    std::tm tm = {};
    std::stringstream ss(starttime);
    ss >> std::get_time(&tm, "%Y%d%m_%H%M%S");
    auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    auto end = chrono::system_clock::now();
    auto dt = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Runtime was " << to_string(dt) << " ms"  << endl;

    return true;
}


bool ACC_SaveData::SaveASCII()
{
    time = getTime();
    std::string rawfn = "./Results/Ascii" + time + ".txt";
	ofstream outfile(rawfn.c_str(), ios::app); 

	std::vector<int> boardsReadyForRead = m_data->data.BoardIndex;
    int NUMSAMP = 256;
    int NUMCH = 30;

    //PARSING
    map<int, vector<unsigned short>> TransferMap;
    int frametype = m_data->data.RawWaveform.size()/m_data->data.BoardIndex.size();
    int n_boards = m_data->data.BoardIndex.size();

    for(int i=0; i<n_boards; i++)
    {
        for(int cj=i*frametype; cj<(i+1)*frametype; cj++)
        {
            TransferMap[m_data->data.BoardIndex.at(i)].push_back(m_data->data.ReceiveData.at(cj));
        }
    }

    map<int,map<int, vector<unsigned short>>> map_data;
	map<int, vector<unsigned short>> map_meta;
    channel_count = 0;

    for(std::map<int, vector<unsigned short>>::iterator it=TransferMap.begin(); it!=TransferMap.end(); ++it)
    {
        if(it->second.size()==16)
        {
            break;
        }

        //Fill ParsedStream with vectors from data
        map_data[it->first] = GetParsedData(it->first,it->second);
        map_meta[it->first] = GetParsedMetaData(it->first,it->second);
    }

	std::string delim = " ";
	for(int enm=0; enm<NUMSAMP; enm++)
	{
		outfile << std::dec << enm << delim;
		for(int bi: boardsReadyForRead)
		{
			if(map_data[bi].size()==0)
			{
				cout << "Mapdata is empty" << endl;
                return false;
			}
			for(int ch=0; ch<NUMCH; ch++)
			{
				outfile << std::dec << (unsigned short)map_data[bi][ch][enm] << delim;
			}
			if(enm<(int)map_meta[bi].size())
			{
				outfile << std::hex << map_meta[bi][enm] << std::dec << delim;
			}else
			{
				outfile << 0 << delim;
			}
		}
		outfile << endl;
	}
	outfile.close();

    boardsReadyForRead.clear();
    map_data.clear();
    map_meta.clear();
}


bool ACC_SaveData::SaveRAW()
{
    if(FileCounter>=EventsPerFile)
    {
        FileCounter=0;
        time = getTime();
    }

    map<int, vector<unsigned short>> TransferMap;
    int frametype = m_data->data.RawWaveform.size()/m_data->data.BoardIndex.size();
    int n_boards = m_data->data.BoardIndex.size();
    for(int i=0; i<n_boards; i++)
    {
        for(int cj=i*frametype; cj<(i+1)*frametype; cj++)
        {
            TransferMap[m_data->data.BoardIndex.at(i)].push_back(m_data->data.ReceiveData.at(cj));
        }
    }

	//Direct raw save of data
	for(std::map<int, vector<unsigned short>>::iterator it=TransferMap.begin(); it!=TransferMap.end(); ++it)
	{
		string rawfn = "./Results/Raw_b" + to_string(it->first) + "_" + time + ".txt";
		ofstream outfile(rawfn.c_str(), ios::app); 
		for(unsigned short k: it->second)
		{
			outfile << hex <<  k << " ";
		}
		outfile << endl;
		outfile.close();
	}

    FileCounter++;
    m_data->TCS.EventCounter++;

    return true;
}


bool ACC_SaveData::SaveStore()
{
    m_data->data.RawWaveform = m_data->data.ReceiveData;
    m_data->Stores["LAPPDStore"]->Set(StoreLabel,m_data->data);
    m_data->Stores["LAPPDStore"]->Save(Path.c_str());
    m_data->Stores["LAPPDStore"]->Delete(); 

    return true;
}


std::map<int,std::vector<unsigned short>> ACC_SaveData::GetParsedData(int boardID, std::vector<unsigned short> buffer)
{
    //Catch empty buffers
	if(buffer.size() == 0){return {};}

	//Prepare the Metadata vector 
	std::map<int,std::vector<unsigned short>> data;
    int NUMCH = 30;
    int NUMPSEC = 5;
    int NUMSAMP = 256;

	//Indicator words for the start/end of the metadata
	const unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11;
	unsigned short endoffile = 0x4321;

	//Empty vector with positions of aboves startword
	vector<int> start_indices = {2, 1554, 3106, 4658, 6210}; 

	//Find the startwords and write them to the vector
	vector<unsigned short>::iterator bit;
	//Fill data map
	for(int i: start_indices)
	{
		//Write the first word after the startword
		bit = buffer.begin() + (i+1);

		//As long as the endword isn't reached copy metadata words into a vector and add to map
		vector<unsigned short> InfoWord;
		while(*bit != endword && *bit != endoffile)
		{
			InfoWord.push_back((unsigned short)*bit);
			if(InfoWord.size()==NUMSAMP)
			{
				data.insert(pair<int, vector<unsigned short>>(channel_count, InfoWord));
				InfoWord.clear();
				channel_count++;
			}
			++bit;
		}	
	}

	return data;
}


std::vector<unsigned short> ACC_SaveData::GetParsedMetaData(int boardID, std::vector<unsigned short> buffer)
{
    //Catch empty buffers
    if(buffer.size() == 0){return {};}

    //Prepare the Metadata vector and helpers
    std::vector<unsigned short> meta;
    int chip_count = 0;
    int NUMCH = 30;
    int NUMPSEC = 5;

    //Indicator words for the start/end of the metadata
    unsigned short endword = 0xFACE;
    unsigned short endoffile = 0x4321;

    //Empty metadata map for each Psec chip <PSEC #, vector with information>
    map<int, vector<unsigned short>> PsecInfo;
    map<int, vector<unsigned short>> PsecTriggerInfo;
    unsigned short CombinedTriggerRateCount;

    //Empty vector with positions of aboves startword
    vector<int> start_indices= {1539, 3091, 4643, 6195, 7747};
    //Find the startwords and write them to the vector
    vector<unsigned short>::iterator bit;

    //Fill the psec info map
    for(int i: start_indices)
    {
        //Write the first word after the startword
        bit = buffer.begin() + (i+1);
        //As long as the endword isn’t reached copy metadata words and add to map
        vector<unsigned short> InfoWord;
        while(*bit != endword && *bit != endoffile && InfoWord.size() < 14)
        {
            InfoWord.push_back(*bit);
            ++bit;
        }
        PsecInfo.insert(pair<int, vector<unsigned short>>(chip_count, InfoWord));
        chip_count++;
    }
    //Fill the psec trigger info map
    for(int chip=0; chip<NUMPSEC; chip++)
    {
        for(int ch=0; ch<NUMCH/NUMPSEC; ch++)
        {
            //Find the trigger data at begin + last_metadata_start + 13_info_words + 1_end_word + 1
            bit = buffer.begin() + start_indices[4] + 13 + 1 + 1 + ch + (chip*(NUMCH/NUMPSEC));
            PsecTriggerInfo[chip].push_back(*bit);
        }
    }
    //Fill the combined trigger
    CombinedTriggerRateCount = buffer[7792];

    //----------------------------------------------------------
    //Start the metadata parsing
    meta.push_back(boardID);
    for(int CHIP=0; CHIP<NUMPSEC; CHIP++)
    {
        meta.push_back((0xDCB0 | CHIP));
        for(int INFOWORD=0; INFOWORD<13; INFOWORD++)
        {
            meta.push_back(PsecInfo[CHIP][INFOWORD]);
        }
        for(int TRIGGERWORD=0; TRIGGERWORD<6; TRIGGERWORD++)
        {
            meta.push_back(PsecTriggerInfo[CHIP][TRIGGERWORD]);
        }
    }
    meta.push_back(CombinedTriggerRateCount);
    meta.push_back(0xeeee);

    return meta;
}