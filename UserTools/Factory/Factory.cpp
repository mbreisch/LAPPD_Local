#include "Factory.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="ACC_DataRead") ret=new ACC_DataRead;
if (tool=="ACC_Evaluate") ret=new ACC_Evaluate;
if (tool=="ACC_Receive") ret=new ACC_Receive;
if (tool=="ACC_SaveData") ret=new ACC_SaveData;
if (tool=="ACC_SetupBoards") ret=new ACC_SetupBoards;
if (tool=="SC_LocalLog") ret=new SC_LocalLog;
if (tool=="SC_Poll_HV") ret=new SC_Poll_HV;
if (tool=="SC_Poll_LV") ret=new SC_Poll_LV;
if (tool=="SC_Poll_Photodiode") ret=new SC_Poll_Photodiode;
if (tool=="SC_Poll_RHT") ret=new SC_Poll_RHT;
if (tool=="SC_Poll_Relays") ret=new SC_Poll_Relays;
if (tool=="SC_Poll_Saltbridge") ret=new SC_Poll_Saltbridge;
if (tool=="SC_Poll_Thermistor") ret=new SC_Poll_Thermistor;
if (tool=="SC_Poll_Timestamp") ret=new SC_Poll_Timestamp;
if (tool=="SC_Poll_Trigger") ret=new SC_Poll_Trigger;
if (tool=="SC_PrintValues") ret=new SC_PrintValues;
if (tool=="SC_Receive") ret=new SC_Receive;
if (tool=="SC_SetConfig") ret=new SC_SetConfig;
if (tool=="SeeConnectedBoards") ret=new SeeConnectedBoards;
return ret;
}
