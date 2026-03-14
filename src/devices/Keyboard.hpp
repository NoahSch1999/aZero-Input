#pragma once
#include <vector>
#include <type_traits>
#include <string>
#include "SDL3/SDL.h"

namespace aZero::Input {
	class Keyboard {
	public:
		Keyboard(const Keyboard&) = delete;
		Keyboard& operator=(const Keyboard&) = delete;

		Keyboard() = default;
		virtual ~Keyboard() = default;

		Keyboard(Keyboard&& other) noexcept {
			this->Move(other);
		}

		Keyboard& operator=(Keyboard&& other) noexcept {
			if (this != &other) {
				this->Move(other);
			}
			return *this;
		}

		bool IsKeyDown(SDL_Scancode scanCode) const { return m_KeyStates[scanCode]; }

		void UpdateKeyStates() { m_KeyStates = SDL_GetKeyboardState(&m_NumKeys); }

		static std::string GetKeyName(SDL_Keycode keyCode) { return SDL_GetKeyName(keyCode); }

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
			std::vector<SDL_KeyboardID> keyboards = Keyboard::GetKeyboards();

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
		void Move(Keyboard& other) {
			m_KeyStates = std::exchange(other.m_KeyStates, nullptr);
			m_NumKeys = std::exchange(other.m_NumKeys, 0);
		}

		// States of SDL_Scancode
		const bool* m_KeyStates = nullptr;
		int32_t m_NumKeys = 0;
	};
}