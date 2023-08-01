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
	auto nextInterrupt = now;
	int intID = 0;
	int cycles = 0;

	while (true)
	{
		cycles = 0;
		while (16667 > cycles)
		{
			cycles += cpu.processInstruction();
			shifter.Process(cpu.IO);
		}

		now = high_resolution_clock::now();
		if (cpu.IE & now>=nextInterrupt)
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
			// two interrupts for a frame time (1/60 seconds)
			nextInterrupt = now + nanoseconds((int)(1000000000.0f/120.0f)); 
		}
	}
}

int main()
{
	Renderer renderer(3.0f);
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
	
	// destroy texture
    SDL_DestroyTexture(renderer.tex);
 
    // destroy renderer
    SDL_DestroyRenderer(renderer.rend);
 
    // destroy window
    SDL_DestroyWindow(renderer.win);

	// close SDL
    SDL_Quit();

	// kill emulation thread
	thr.~thread();

	return 0;
}