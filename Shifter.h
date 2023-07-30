#pragma once
#include "IOcontroller.h"
class Shifter
{
private:
	uint16_t reg;
	int offset;
public:
	Shifter();
	~Shifter();
	void Process(IOcontroller& io);
};

