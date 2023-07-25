#ifndef ACC_Evaluate_H
#define ACC_Evaluate_H

#include <string>
#include <iostream>

#include "Tool.h"


using namespace std;

/**
 * \class ACC_Evaluate
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
*/

class ACC_Evaluate: public Tool {


 public:

  ACC_Evaluate(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.


 private:

    std::string seperator;
    std::vector<int> LAPPD_on_ACC;
    int LAPPD_ID;
    int Port_0;
    int Port_1;

    void Print_ACC_IF(std::vector<unsigned short> accif);
    void Print_Buffer_Debug(std::vector<unsigned short> accif);

    ifstream logfile;
    int loglength;


};


#endif
