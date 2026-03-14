#pragma once
#include "DeviceManager.hpp"

namespace aZero::Input {
	inline void Init(SDL_InitFlags flags = SDL_INIT_GAMEPAD) {
		SDL_Init(flags);
	}

	inline void Shutdown() {
		SDL_Quit();
	}
}