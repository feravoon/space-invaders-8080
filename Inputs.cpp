#include "Inputs.h"
#include "SDL2/SDL.h"


bool Inputs::Refresh(IOcontroller& io)
{
	SDL_Event event;
    const Uint8* keystate;
	bool close = false;

	if(SDL_PollEvent(&event))
	{
		if(event.type == SDL_QUIT)
		{
			close = true;
		}
	}

    keystate = SDL_GetKeyboardState(NULL);

	coinInfo = true;
	coin = keystate[SDL_SCANCODE_C];

	p1start = keystate[SDL_SCANCODE_1];
	p1left = keystate[SDL_SCANCODE_LEFT];
	p1right = keystate[SDL_SCANCODE_RIGHT];
	p1fire = keystate[SDL_SCANCODE_F];

	uint8_t port1 = (uint8_t)((coin ? 0 : 1) | ((p2start ? 1 : 0) << 1) | ((p1start ? 1 : 0) << 2) | (1 << 3) | ((p1fire ? 1 : 0) << 4) | ((p1left ? 1 : 0) << 5) | ((p1right ? 1 : 0) << 6));
	uint8_t port2 = (uint8_t)(numberOfLives | ((tilt ? 1 : 0) << 2) | ((bonusLife ? 1 : 0) << 3) | ((p2fire ? 1 : 0) << 4) | ((p2left ? 1 : 0) << 5) | ((p2right ? 1 : 0) << 6) | ((coinInfo ? 1 : 0) << 7));

	io.I[1] = port1;
	io.I[2] = port2;

	return close;
}

Inputs::Inputs()
{
	numberOfLives = 0b11; // 00 = 3 ships,  10 = 5 ships, 01 = 4 ships,  11 = 6 ships
	bonusLife = false; // bonus life at true:1000, false:1500
}

Inputs::~Inputs()
{
}
