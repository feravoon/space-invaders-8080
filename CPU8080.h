#pragma once
#include <cstdint>
#include "Memory.h"
#include "IOcontroller.h"
class CPU8080
{
private:
	uint8_t A, B, C, D, E, H, L;
	uint16_t SP;
	bool Z, S, P, CY, AC;
	uint8_t getM();
	void setM(uint8_t value);
	uint16_t getBC(), getDE(), getHL();
	void setBC(uint16_t BC), setDE(uint16_t DE), setHL(uint16_t HL);
	uint8_t getFlags();
	void setFlags(uint8_t value);
	void updateFlagsArithmetic(int res);
	void updateFlagsLogic();
	void updateFlagsZSP(uint8_t val);
	static bool parity(int x, int size);
	static const int cycles[];
	static const int byteLengths[];

public:
	bool printOutput;
	uint16_t PC;
	bool IE;
	Memory memory;
	IOcontroller IO;
	CPU8080();
	int processInstruction();
	int disassemble8080Op();
	void generateInterrupt(int intID);


};