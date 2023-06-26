#include "Factory.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="ACC_DataRead") ret=new ACC_DataRead;
if (tool=="ACC_Receive") ret=new ACC_Receive;
if (tool=="ACC_SaveData") ret=new ACC_SaveData;
if (tool=="ACC_SetupBoards") ret=new ACC_SetupBoards;
if (tool=="SeeConnectedBoards") ret=new SeeConnectedBoards;
if (tool=="ACC_Evaluate") ret=new ACC_Evaluate;
return ret;
}
