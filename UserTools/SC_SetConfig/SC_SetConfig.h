#ifndef SC_SetConfig_H
#define SC_SetConfig_H

#include <string>
#include <iostream>
#include <fstream>

#include "Tool.h"

using namespace std;
/**
 * \class Set
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class SC_SetConfig: public Tool {


 public:

    SC_SetConfig(); ///< Simple constructor
    bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
    bool Execute(); ///< Executre function used to perform Tool perpose. 
    bool Finalise(); ///< Finalise funciton used to clean up resorces.

    bool Setup();
    bool TurnOn();
    bool TurnOff();
    bool Update();

    void Control_Relay();
    void Control_HV_Volts();
    void Control_HV_State();
    void Control_LV_State();
    void Control_Trigger_DAC0();
    void Control_Trigger_DAC1();

 private:

    bool flag;
    int retval;
    int LAPPD_ID;

};


#endif
