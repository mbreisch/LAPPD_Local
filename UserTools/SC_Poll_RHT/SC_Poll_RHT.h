#ifndef SC_Poll_RHT_H
#define SC_Poll_RHT_H

#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>

#include "Tool.h"

using namespace std;

/**
 * \class Poll
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
* Contact: b.richards@qmul.ac.uk
*/
class SC_Poll_RHT: public Tool {


    public:

        SC_Poll_RHT(); ///< Simple constructor
        bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
        bool Execute(); ///< Executre function used to perform Tool perpose. 
        bool Finalise(); ///< Finalise funciton used to clean up resorces.

        bool TEMPCHK();
        bool HUMIDITYCHK();
        bool HardShutdown(float errortype);

    private:

        int HFlag;
        int CheckDelay;
        float tool_humidity_limit;
};


#endif
