#include "Shifter.h"

Shifter::Shifter() : reg(), offset()
{
}


Shifter::~Shifter()
{
}

void Shifter::Process(IOcontroller& io)
{

	if (io.newOutput[4]) // A new output to port 4 from cpu means writing a new value to register
	{
		uint8_t value = io.O[4];
		reg >>= 8;
		reg = (reg & 0x00ff) | (value << 8);
		io.I[3] = (uint8_t)((reg >> (8 - offset)) & 0x0ff);
		io.newOutput[4] = false;
	}

	if (io.newOutput[2]) // A new output to port 2 from cpu sets the offset value
	{
		uint8_t value = io.O[2];
		offset = value & 0x07;
		io.I[3] = (uint8_t)((reg >> (8 - offset)) & 0x0ff);
		io.newOutput[2] = false;
	}

}