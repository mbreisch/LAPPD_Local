#ifndef _ACDC_H_INCLUDED
#define _ACDC_H_INCLUDED

#include <iostream>

class ACDC
{
public:
	ACDC(); //constructor
	~ACDC(); //deconstructor

	//----------local return functions
	int getBoardIndex(); //get the current board index from the acdc

	//----------local set functions
	void setBoardIndex(int bi); // set the board index for the current acdc

private:
	//----------all neccessary global variables
	int boardIndex; //var: represents the boardindex for the current board

};

#endif
