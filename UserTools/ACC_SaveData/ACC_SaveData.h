#ifndef ACC_SaveData_H
#define ACC_SaveData_H

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <iomanip>

#include "Tool.h"

using namespace std;


/**
 * \class ACC_SaveData
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
*/

class ACC_SaveData: public Tool 
{
    public:

        ACC_SaveData(); ///< Simple constructor
        bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
        bool Execute(); ///< Executre function used to perform Tool perpose. 
        bool Finalise(); ///< Finalise funciton used to clean up resorces.

    private:

        int SaveMode;
        std::string Path;
        std::string StoreLabel;
        std::string time;
        std::string starttime;

        int FileCounter;
        int EventsPerFile;
        int MaxNumberEvents;
        int channel_count;

        bool SaveASCII();
        bool SaveRAW();
        bool SaveStore();

        std::vector<unsigned short> GetParsedData(int boardID, std::vector<unsigned short> buffer);
        std::vector<unsigned short> GetParsedMetaData(int boardID, std::vector<unsigned short> buffer);

        std::string getTime()
        {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y%d%m_%H%M%S");
            return ss.str();
        }

};


#endif
