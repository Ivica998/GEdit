#pragma once
#include "utils.h"

class Debug {
public:

	static bool Ready;
	static int rate;

	static void Log(std::string str) {
		if (Ready) {
			std::cout << str;
		}
	}
	static void CountTillReady() {
		static int counter = 0;
		counter++;
		Ready = false;
		if(counter > rate)
		{
			counter = 0;
			Ready = true;
		}
	}
};
typedef Debug dbg;