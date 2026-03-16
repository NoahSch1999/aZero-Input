#pragma once
#include <vector>
#include <type_traits>
#include <string>
#include "SDL3/SDL.h"

namespace aZero::Input {

	// NOTE: I dont like this class being pretty much a namespace since we cant have multiple keyboards. 
	// But its easier to have them under a single "namespace" than using the raw C function, so this is still ok...
	class Keyboard {
	public:
		Keyboard(const Keyboard&) = delete;
		Keyboard& operator=(const Keyboard&) = delete;

		Keyboard() = default;

		virtual ~Keyboard() = default;

		Keyboard(Keyboard&& other) noexcept = default;

		Keyboard& operator=(Keyboard&& other) noexcept = default;

		static bool IsKeyDown(SDL_Scancode scanCode) { return m_KeyStates[scanCode]; }

		static void UpdateState() { m_KeyStates = SDL_GetKeyboardState(&m_NumKeys); }

		static std::string GetKeyName(SDL_Keycode keyCode) { return SDL_GetKeyName(keyCode); }

		static bool IsConnected() { return SDL_HasKeyboard(); }

		static std::vector<SDL_KeyboardID> GetKeyboards() {
			int32_t count = 0;
			SDL_KeyboardID* keyboardsPtr = SDL_GetKeyboards(&count);

			std::vector<SDL_KeyboardID> keyboards;
			keyboards.reserve(count);
			for (int32_t i = 0; i < count; i++) {
				keyboards.emplace_back(keyboardsPtr[i]);
			}
			SDL_free(keyboardsPtr);

			return keyboards;
		}

		static std::vector<std::string> GetKeyboardNames() {
			const std::vector<SDL_KeyboardID> keyboards = Keyboard::GetKeyboards();

			std::vector<std::string> names;
			names.reserve(keyboards.size());
			for (SDL_KeyboardID id : keyboards) {
				if (id == 0)
					continue;
				names.emplace_back(SDL_GetKeyboardNameForID(id));
			}
			return names;
		}

	private:
		// States of SDL_Scancode
		inline static const bool* m_KeyStates = nullptr;
		inline static int32_t m_NumKeys = 0;
	};
}