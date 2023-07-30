#include <stdio.h>
#include "CPU8080.h"
#include "Renderer.h"
#include "Shifter.h"
#include "Inputs.h"
#include <thread>
#include <cmath>
using namespace std::chrono;

CPU8080 cpu;
Shifter shifter;
Inputs inputs;
std::atomic<bool> paintNow{false};

void emulateMachine()
{
	auto now = high_resolution_clock::now();
	auto workDuration = high_resolution_clock::now() - now;
	int intID = 0;
	int cycles = 0;

	while (true)
	{
		now = high_resolution_clock::now();
		cycles = 0;

		while (16667 > cycles)
		{
			cycles += cpu.processInstruction();
			shifter.Process(cpu.IO);
		}

		if (cpu.IE)
		{
			if (intID == 1)
			{
				paintNow = true;
				cpu.generateInterrupt(1);
				intID = 2;
			}
			else
			{
				cpu.generateInterrupt(2);
				intID = 1;
			}
			
		}
		workDuration = high_resolution_clock::now() - now;
		std::this_thread::sleep_for(milliseconds((int)round(1000.0f / 120.0f)) - workDuration);
	}
}

int main() {
	Renderer renderer;
   	cpu = CPU8080(); // Initialize the CPU
	cpu.printOutput = false; // for printing debug output to console
	bool close = false;
	
	// Read Space Invaders ROM files into memory
	cpu.memory.ReadFileIntoMemoryAt("invaders/invaders.h", 0x0000);
	cpu.memory.ReadFileIntoMemoryAt("invaders/invaders.g", 0x0800);
	cpu.memory.ReadFileIntoMemoryAt("invaders/invaders.f", 0x1000);
	cpu.memory.ReadFileIntoMemoryAt("invaders/invaders.e", 0x1800);

	std::thread thr(emulateMachine);

	while(!close)
	{		
		if(paintNow)
		{
			paintNow = false;
			close = inputs.Refresh(cpu.IO);
			renderer.render(cpu.memory);
		}
	}
	return 0;
}

