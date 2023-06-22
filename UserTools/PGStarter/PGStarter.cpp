#include "PGStarter.h"
#include "JsonParser.h"

PGStarter::PGStarter():Tool(){}


bool PGStarter::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();
  
  m_data= &data;
  m_log= m_data->Log;
  
  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  m_data->postgres_helper.SetVerbosity(m_verbose);
  
  // configure the PGHelper to dump the configurations it retrieves
  bool dumpconfigs = false;
    std::ofstream* textout = nullptr;
  if(m_variables.Get("dumpconfigs",dumpconfigs) && dumpconfigs){
    m_data->postgres_helper.SetDumpConfigs(dumpconfigs);
    textout = new std::ofstream("PGStarter_config.txt");
    if(!textout->is_open()){
      std::cerr<<"failed to open 'PGStarter_config.txt' for writing"<<std::endl;
    }
  }
  
  // if we're the main DAQ we will make a new entry in the run table
  // at the start of each run. But only the main DAQ toolchain should do this.
  std::string systemname="";
  daqtoolchain=false;
  if(!m_data->vars.Get("SystemName",systemname)){
    Log("PGStarter did not find a SystemName in ToolChainConfig!",v_error,verbosity);
    return false;
  } else {
    daqtoolchain = (systemname=="daq");
  }
  
  // get config file of settings for connecting to the database
  std::string pgsettingsfile="";
  get_ok = m_variables.Get("pgsettingsfile",pgsettingsfile);
  if(not get_ok){
    Log("PGStarter couldn't find pgsettingsfile in m_variables! Need to know postgres connection settings!",
        v_warning,verbosity);
    // i guess we can continue with defaults of everything
  }
  
  get_ok = m_data->pgclient.Initialise(pgsettingsfile);
  if(not get_ok){
    Log("PGClient failed to Initialise!",v_error,verbosity);
    return false;
  }
  
  // after Initilising the pgclient needs ~15 seconds for the middleman to connect
  std::this_thread::sleep_for(std::chrono::seconds(15));
  // hopefully the middleman has found us by now
  
  // for now until the PGHelper does it for us we need to extract the results from
  // the returned json using a Store
  Store resultstore;
  
  // make a query to get the run number
  // TODO update the PGHelper to work with the new PGClient to make these cleaner
  std::string resultstring="";
  std::string err="";
  std::string dbname="rundb";
  std::string query_string="SELECT max(runnum) AS run FROM run;";
  int timeout_ms=5000;
  get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
  if(not get_ok){
    Log("PGStarter failed to get run number with return "+std::to_string(get_ok)
       +" and error "+err,v_error,verbosity);
    return false;
  }
  int runnum=0;
  resultstore.JsonParser(resultstring);
  get_ok = resultstore.Get("run",runnum);
  if(not get_ok){
    Log("PGStarter failed to get runnum from query response '"+resultstring+"'",v_error,verbosity);
    return false;
  }
  
  bool new_run=true;
  // see if the max run num is the same as the run already in the datamodel
  if(runnum>0 && m_data->run == runnum){
    // we've already initialised the datamodel run number - so if we're
    // calling Initialise again, it's for a new subrun
    new_run=false;
  };
  m_data->run = runnum;
  
  // make a query to get the subrrun for this runnum
  resultstring="";
  err="";
  query_string="SELECT max(subrunnum) AS subrun FROM run WHERE runnum="+std::to_string(runnum)+";";
  get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
  if(not get_ok){
    Log("PGStarter failed to get subrun number with return "+std::to_string(get_ok)
       +" and error "+err,v_error,verbosity);
    return false;
  }
  int subrunnum=0;
  resultstore.JsonParser(resultstring);
  get_ok = resultstore.Get("subrun",subrunnum);
  if(not get_ok){
    Log("PGStarter failed to get subrunnum from query response '"+resultstring+"'",v_error,verbosity);
    return false;
  }
  m_data->subrun = subrunnum;

  // if it's a new run we need to look up the run configuration.
  // if we're the main DAQ we do this from the command line args
  int runconfig;
  std::string runtype="";
  if(daqtoolchain){
    
    // get type of run from command line arg 1
    m_data->vars.Get("$1",runtype);
    // we need to extract the runtype from the command line argument.
    // this may be something like 'Beam', but may also be './Beam'
    // or './configfiles/Beam/ToolChainConfig', so strip as appropriate
    
    // if '/ToolChainConfig' found, strip it
    size_t apos=runtype.find("/ToolChainConfig");
    if(apos!=std::string::npos){
        runtype=runtype.substr(0,apos);
    }
    
    // if '/' found, strip off it and everything before it
    apos=runtype.find_last_of("/");
    if(apos!=std::string::npos){
        runtype=runtype.substr(apos+1,std::string::npos);
    }
    
    m_data->RunType=runtype;
    
    // we then need to look up the runconfig ID of the latest version of this run type.
    // the run config ID uniquely identifies the run configuration,
    // and is needed by other Tools to get their configuration parameters
    resultstring="";
    err="";
    //query_string="SELECT version FROM runconfig INNER JOIN run ON run.runconfig = runconfig.id WHERE name='"
    //             + m_data->RunType+"' AND runnum = (SELECT max(runnum) FROM run)";
    query_string="SELECT id FROM runconfig WHERE name='"+m_data->RunType+"' AND version="
                 "(SELECT max(version) FROM runconfig WHERE name='"+m_data->RunType+"');";
    get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
    if(not get_ok){
      Log("PGStarter failed to get runtype id and version for type "+m_data->RunType
         +" with return "+std::to_string(get_ok)+" and error "+err,v_error,verbosity);
      return false;
    }
    resultstore.JsonParser(resultstring);
    get_ok = resultstore.Get("id",runconfig);
    if(not get_ok){
      Log("PGStarter failed to get run config from query response '"+resultstring+"'",v_error,verbosity);
      return false;
    }
    m_data->RunConfig=runconfig;
    
    // we make a new entry in the run table at the start of each new run or subrun;
    // increment either runnum or subrunnum as appropriate
    if(new_run){
      ++runnum;
      subrunnum=0;
    } else {
      ++subrunnum;
    }
    m_data->run = runnum;
    m_data->subrun = subrunnum;
    
    // and create a new record in the run table for this run/subrun
    query_string = "INSERT INTO run ( runnum, subrunnum, start, runconfig ) VALUES ( "
                   +std::to_string(m_data->run)+", "+std::to_string(m_data->subrun)
                   +", 'now()', "+std::to_string(m_data->RunConfig)+");";
    get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
    if(not get_ok){
      Log("PGStarter failed to make new entry with query '"+query_string+"'"
          ", returned with error '"+err+"'",v_error,verbosity);
      return false;
    }
    
    
  } else if(new_run){
    
    // if we are not the main toolchain we need to get the run configuration id
    // corresponding to the current run (i.e. latest run entry)
    // (limit 1 needed if we have subruns)
    query_string = "SELECT runconfig FROM run WHERE runnum=(SELECT MAX(runnum) FROM run) LIMIT 1";
    get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
    if(not get_ok){
      Log("PGStarter failed to get runconfig number with return "+std::to_string(get_ok)
         +" and error "+err,v_error,verbosity);
      return false;
    }
    resultstore.JsonParser(resultstring);
    get_ok = resultstore.Get("runconfig",runconfig);
    if(not get_ok){
      Log("PGStarter failed to get runconfig from query response '"+resultstring+"'",v_error,verbosity);
      return false;
    }
    m_data->RunConfig=runconfig;
  }
  
  // next we use the runconfig to get the system configuration id from the runconfig entry
  query_string = "SELECT "+systemname+" FROM runconfig WHERE id="+std::to_string(m_data->RunConfig);
  get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
  if(not get_ok){
    Log("PGStarter failed to get "+systemname+" config ID with return "+std::to_string(get_ok)
       +" and error "+err,v_error,verbosity);
    return false;
  }
  resultstore.JsonParser(resultstring);
  int systemconfigid=0;
  get_ok = resultstore.Get(systemname,systemconfigid);
  if(not get_ok){
    Log("PGStarter failed to get '"+systemname+"' from query response '"+resultstring+"'",v_error,verbosity);
    return false;
  }
  
  // finally we get the toolsconfig for this system
  query_string = "SELECT toolsconfig FROM "+systemname+" WHERE id="+std::to_string(systemconfigid);
  get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
  if(not get_ok){
    Log("PGStarter failed to get toolsconfig with return "+std::to_string(get_ok)
       +" and error "+err,v_error,verbosity);
    return false;
  }
  
  // the toolsconfig itself is a json representing a map of Tool to config file version number.
  // a combination of system name, tool name and version number uniquely identifies a configuration string
  // use a proper json parser to get the json string from the query response
  JSONP parser;
  BoostStore* toolsconfigbstore = new BoostStore{};
  parser.Parse(resultstring, *toolsconfigbstore);
  std::string toolsconfigstring;
  toolsconfigbstore->Get("toolsconfig",toolsconfigstring);
  // then parse that json in turn for the version numbers of each Tool's configuration
  parser.Parse(toolsconfigstring, *toolsconfigbstore);
  // we put that info in the datamodel. it'll be used by the PGHelper when each Tool
  // asks for its Tool configuration
  if(m_data->Stores.count("ToolsConfig")){
    BoostStore* oldconfig = m_data->Stores.at("ToolsConfig");
    delete oldconfig;
  }
  m_data->Stores["ToolsConfig"] = toolsconfigbstore;
  
  if(textout){
    *textout<<"SYSTEM: "<<systemname<<std::endl;
    *textout<<"DAQ TOOLCHAIN: "<<daqtoolchain<<std::endl;
    *textout<<"RUN: "<<m_data->run<<std::endl;
    *textout<<"SUBRUN: "<<m_data->subrun<<std::endl;
    *textout<<"NEW RUN:" <<new_run<<std::endl;
    *textout<<"RUNTYPE: "<<m_data->RunType<<std::endl;
    *textout<<"RUNCONFIG ID: "<<m_data->RunConfig<<std::endl;
    *textout<<"SYSTEMCONFIG ID: "<<systemconfigid<<std::endl;
    *textout<<"TOOLSCONFIG: "<<toolsconfigstring<<std::endl;
  }
  
  Log("This run is run number "+std::to_string(m_data->run)
     +", subrun "+std::to_string(m_data->subrun),
     v_message,verbosity);
  
  return true;
}


bool PGStarter::Execute(){
  
  try {
    if(m_data->reinit){
      Log("PGStarter reinit set: Finalising...",v_message,verbosity);
      Finalise();
      Log("...Initialising...",v_message,verbosity);
      Initialise("",*m_data);
      Log("PGStarter reinit done",v_message,verbosity);
    }
  } catch(std::exception& e){
    std::cout<<"PGStarter::Execute experienced an error '"<<e.what()<<"' reinitialising!"<<std::endl;
    return false;
  }
  
  return true;
}


bool PGStarter::Finalise(){

  if(daqtoolchain){
	  // update the record in the run table for this run/subrun with the number of events taken
	  std::string dbname="rundb";
	  std::string resultstring;
	  std::string err;
	  int timeout_ms=5000;
	  std::string query_string = "UPDATE run SET stop='now()', numevents="+std::to_string(m_data->triggers["VME"])
	                            +" WHERE runnum="+std::to_string(m_data->run)
	                            +" AND subrunnum="+std::to_string(m_data->subrun)+";";
	  get_ok = m_data->pgclient.SendQuery(dbname, query_string, &resultstring, &timeout_ms, &err);
	  if(not get_ok){
	    Log("PGStarter failed to update end of run info with query '"+query_string+"'"
	        ", returned with error '"+err+"'",v_error,verbosity);
	    return false;
	  }
  }
  
  m_data->pgclient.Finalise();
  
  return true;
}
