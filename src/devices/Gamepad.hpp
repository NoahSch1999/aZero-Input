#pragma once
#include <vector>
#include <array>
#include <type_traits>
#include <tuple>
#include <string>
#include <stdexcept>
#include "SDL3/SDL.h"

namespace aZero::Input {
	class Gamepad {
	public:
		Gamepad(const Gamepad&) = delete;
		Gamepad& operator=(const Gamepad&) = delete;

		SDL_Gamepad* GetSDLGamepad() const { return m_Gamepad; }
		SDL_JoystickID GetSDLJoystickID() const { return m_Id; }

		Gamepad()
			:m_ButtonDownStates{ false }{ }

		explicit Gamepad(SDL_JoystickID id) 
			:m_ButtonDownStates{ false } { this->Init(id); }

		void Init(SDL_JoystickID id) {
			if (!SDL_IsGamepad(id)) {
				throw std::runtime_error("Controller not supported as SDL_Gamepad");
			}

			m_Id = id;
		}

		virtual ~Gamepad() = default;

		Gamepad(Gamepad&& other) noexcept {
			this->Move(other);
		}

		Gamepad& operator=(Gamepad&& other) noexcept {
			if (this != &other) {
				this->Move(other);
			}
			return *this;
		}

		bool Connect() {
			m_Gamepad = SDL_OpenGamepad(m_Id);
			return m_Gamepad != NULL;
		}

		void Disconnect() {
			m_ButtonDownStates.fill(false);
			SDL_CloseGamepad(m_Gamepad);
		}

		void ToggleEnabled(bool enabled) { SDL_SetGamepadEventsEnabled(enabled); }

		void MapButtons(/*data structure describing buttons and their mappings*/) {
			// TODO: Impl easy-to-use button mapping
		}

		bool StartRumbleController(uint16_t lowFreq, uint16_t highFreq, uint32_t durationMS) {
			const SDL_GamepadType type = this->GetType();
			if (type == SDL_GAMEPAD_TYPE_PS3 || type == SDL_GAMEPAD_TYPE_PS4 || type == SDL_GAMEPAD_TYPE_PS5
				|| type == SDL_GAMEPAD_TYPE_XBOX360 || type == SDL_GAMEPAD_TYPE_XBOXONE
				|| type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO || type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO)
			{
				return SDL_RumbleGamepad(m_Gamepad, lowFreq, highFreq, durationMS);
			}
			return false;
		}

		bool StartRumbleTriggers(uint16_t leftRumble, uint16_t rightRumble, uint32_t durationMS) {

			const SDL_GamepadType type = this->GetType();
			if (type == SDL_GAMEPAD_TYPE_XBOXONE)
			{
				return SDL_RumbleGamepadTriggers(m_Gamepad, leftRumble, rightRumble, durationMS);
			}
			return false;
		}

		bool SetLED(uint8_t red, uint8_t green, uint8_t blue) {
			const SDL_GamepadType type = this->GetType();
			if (type == SDL_GAMEPAD_TYPE_PS4 || type == SDL_GAMEPAD_TYPE_PS5) {
				return SDL_SetGamepadLED(m_Gamepad, red, green, blue);
			}
			return false;
		}

		void ProcessEvent(const SDL_Event& event) {
			const SDL_JoystickID id = this->GetSDLJoystickID();
			if (id == event.gbutton.which) {
				if (event.gbutton.type == SDL_EVENT_GAMEPAD_BUTTON_UP || event.gbutton.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
					this->HandleButtonState(event.gbutton);
				}
			}
		}

		bool IsConnected() const { return SDL_GamepadConnected(m_Gamepad); }

		std::string GetName() const { return SDL_GetGamepadName(m_Gamepad); }
		std::string GetPath() const { return SDL_GetGamepadPath(m_Gamepad); }

		SDL_GamepadType GetType() const { return SDL_GetGamepadType(m_Gamepad); }
		SDL_JoystickConnectionState GetConnectionType() const { return SDL_GetGamepadConnectionState(m_Gamepad); }

		bool IsButtonDown(SDL_GamepadButton button) const { return m_ButtonDownStates[button]; }
		bool GetButtonState(SDL_GamepadButton button) const { return SDL_GetGamepadButton(m_Gamepad, button); }
		std::array<bool, SDL_GAMEPAD_BUTTON_COUNT> GetButtonDownStates() const { return m_ButtonDownStates; }
		SDL_GamepadButtonLabel GetButtonLabel(SDL_GamepadButton button) const { return SDL_GetGamepadButtonLabel(m_Gamepad, button); }

		std::vector<SDL_GamepadBinding> GetBindings() const {
			int32_t count = 0;
			SDL_GamepadBinding** bindingsPtr = SDL_GetGamepadBindings(m_Gamepad, &count);

			std::vector<SDL_GamepadBinding> bindings(count);
			for (uint32_t i = 0; i < count; i++) {
				bindings[i] = *bindingsPtr[i];
			}
			SDL_free(bindingsPtr);

			return bindings;
		}

		std::string GetSDLGUID() const {
			SDL_GUID guid = SDL_GetGamepadGUIDForID(m_Id);
			std::string buffer;
			buffer.resize(33);
			SDL_GUIDToString(guid, buffer.data(), buffer.length());
			return buffer;
		}

		std::tuple<SDL_PowerState, int32_t> GetBatteryState() const {
			int32_t percent = 0;
			const SDL_PowerState state = SDL_GetGamepadPowerInfo(m_Gamepad, &percent);
			return std::make_tuple(state, percent);
		}

		static std::vector<SDL_JoystickID> GetGamepads() {
			int32_t count = 0;
			SDL_JoystickID* joysticksPtr = SDL_GetGamepads(&count);

			std::vector<SDL_JoystickID> joysticks;
			joysticks.reserve(count);
			for (int32_t i = 0; i < count; i++) {
				if (joysticksPtr[i] == 0)
					continue;
				joysticks.emplace_back(joysticksPtr[i]);
			}
			SDL_free(joysticksPtr);

			return joysticks;
		}
		
	private:
		void HandleButtonState(const SDL_GamepadButtonEvent& button) {
			if (button.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
				m_ButtonDownStates[button.button] = false;
			}
			else if (button.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
				m_ButtonDownStates[button.button] = true;
			}
		}

		void Move(Gamepad& other) {
			m_Gamepad = std::exchange(other.m_Gamepad, nullptr);
			m_ButtonDownStates = std::move(other.m_ButtonDownStates);
			m_Id = std::exchange(other.m_Id, 0);
		}

		SDL_Gamepad* m_Gamepad = nullptr;
		SDL_JoystickID m_Id = 0;

		// L2 and R2 are treated as analog buttons so they're not here.
		std::array<bool, SDL_GAMEPAD_BUTTON_COUNT> m_ButtonDownStates;
	};
}