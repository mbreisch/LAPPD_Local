#ifndef ACC_DataRead_H
#define ACC_DataRead_H

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>
#include <queue>

#include "Tool.h"

using namespace std;

#define UTCCONV 18000000

/**
 * \class ListenForData
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class ACC_DataRead: public Tool {


 public:

  ACC_DataRead(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose.
  bool Finalise(); ///< Finalise funciton used to clean up resorces.

  map<int,unsigned short> PreviousBuffer;
  vector<int> LAPPD_on_ACC;
  int LAPPD_ID;
  int Port_0;
  int Port_1;

 private:

  std::string localconfigfile;




};


#endif
