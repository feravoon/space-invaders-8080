#pragma once
#include "IOcontroller.h"
class Inputs
{
private:
	bool p1start, p1fire, p1left, p1right, p2start, p2fire, p2left, p2right, coin, coinInfo, tilt;
	uint8_t numberOfLives;
	bool bonusLife;
public:
	bool Refresh(IOcontroller& io);
	Inputs();
	~Inputs();
};

