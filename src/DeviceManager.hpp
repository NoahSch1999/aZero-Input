#pragma once
#include "DeviceController.hpp"
#include "devices/gamepad.hpp"
#include "devices/Keyboard.hpp"
#include "devices/Mouse.hpp"

namespace aZero::Input {
	using GamepadListener = DeviceListener<Gamepad>;
	using KeyboardListener = DeviceListener<Keyboard>;
	using MouseListener = DeviceListener<Mouse>;

	struct DeviceManager
	{
	public:
		GamepadListener ListenGamepad(const std::string& guid, ListenerCallbacks<Gamepad>&& callbacks)
		{
			for (auto& [id, gamepad] : m_Gamepads)
			{
				if (guid == gamepad->m_Device.GetSDLGUID())
				{
					const DeviceListenerID id = m_NextListenerId++;
					gamepad->m_Listeners[id] = ListenerCallbacks<Gamepad>(std::forward<ListenerCallbacks<Gamepad>>(callbacks));
					return GamepadListener(gamepad, gamepad->m_Device, id, guid);
				}
			}
			
			return GamepadListener();
		}

		KeyboardListener ListenKeyboard(ListenerCallbacks<Keyboard>&& callbacks)
		{
			if (!m_Keyboard)
			{
				return KeyboardListener();
			}
			const DeviceListenerID id = m_NextListenerId++;
			m_Keyboard->m_Listeners[id] = ListenerCallbacks<Keyboard>(std::forward<ListenerCallbacks<Keyboard>>(callbacks));
			return KeyboardListener(m_Keyboard, m_Keyboard->m_Device, id, "");
		}

		MouseListener ListenMouse(ListenerCallbacks<Mouse>&& callbacks)
		{
			if (!m_Mouse)
			{
				return MouseListener();
			}
			const DeviceListenerID id = m_NextListenerId++;
			m_Mouse->m_Listeners[id] = ListenerCallbacks<Mouse>(std::forward<ListenerCallbacks<Mouse>>(callbacks));
			return MouseListener(m_Mouse, m_Mouse->m_Device, id, "");
		}

		void UpdateDeviceStates()
		{
			Keyboard::UpdateState();
			Mouse::UpdateState();
		}

		void ProcessEvent(const SDL_Event& event)
		{
			if (m_Keyboard)
			{
				// TODO: Look into this. We support one keyboard, but some keyboards have multiple ids for a single keyboard...
				if (event.kdevice.type == SDL_EVENT_KEYBOARD_REMOVED && Input::Keyboard::IsConnected())
				{
					for (auto& [id, callbacks] : m_Keyboard->m_Listeners) {
						callbacks.m_OnDisconnect(event, m_Keyboard->m_Device);
					}
					m_Keyboard.reset();
				}
				else if (event.key.type == SDL_EVENT_KEY_UP || event.key.type == SDL_EVENT_KEY_DOWN)
				{
					for (auto& [id, callbacks] : m_Keyboard->m_Listeners) {
						callbacks.m_OnEvent(event, m_Keyboard->m_Device);
					}
				}
			}
			else
			{
				if (event.kdevice.type == SDL_EVENT_KEYBOARD_ADDED)
				{
					m_Keyboard = std::make_shared<DeviceController<Keyboard>>(Keyboard());
				}
			}

			if (m_Mouse)
			{
				// TODO: Look into this. We support one mouse, but some mice have multiple ids for a single mouse...
				if (event.kdevice.type == SDL_EVENT_MOUSE_REMOVED && Input::Mouse::IsConnected())
				{
					for (auto& [id, callbacks] : m_Mouse->m_Listeners) {
						callbacks.m_OnDisconnect(event, m_Mouse->m_Device);
					}
					m_Mouse.reset();
				}
				else if (
				/* Button event   */ event.mdevice.type == SDL_EVENT_MOUSE_BUTTON_UP || event.mdevice.type == SDL_EVENT_MOUSE_BUTTON_DOWN
				/* Move   event   */ || event.mdevice.type == SDL_EVENT_MOUSE_MOTION
				/* Scroll event   */ || event.mdevice.type == SDL_EVENT_MOUSE_WHEEL)
				{
					for (auto& [id, callbacks] : m_Mouse->m_Listeners) {
						callbacks.m_OnEvent(event, m_Mouse->m_Device);
					}
				}
			}
			else
			{
				if (event.mdevice.type == SDL_EVENT_MOUSE_ADDED)
				{
					m_Mouse = std::make_shared<DeviceController<Mouse>>(Mouse());
				}
			}
			
			// TODO: Add a small delta so we dont call axis events a bajillion times...
			if (
				/* Button event   */ event.gbutton.type == SDL_EVENT_GAMEPAD_BUTTON_UP || event.gbutton.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN
				/* Axis event     */ || event.gaxis.type == SDL_EVENT_GAMEPAD_AXIS_MOTION
				/* Touchpad event */ || event.gtouchpad.type == SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN || event.gtouchpad.type == SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION || event.gtouchpad.type == SDL_EVENT_GAMEPAD_TOUCHPAD_UP
				/* Device event   */ || event.gdevice.type == SDL_EVENT_GAMEPAD_REMAPPED || event.gdevice.type == SDL_EVENT_GAMEPAD_UPDATE_COMPLETE || event.gdevice.type == SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED
				)
			{
				if (auto gamepadIter = m_Gamepads.find(event.gbutton.which); gamepadIter != m_Gamepads.end())
				{
					Gamepad& gamepad = gamepadIter->second->m_Device;
					auto& listeners = gamepadIter->second->m_Listeners;

					gamepad.ProcessEvent(event);
					for (auto& [id, callbacks] : listeners) {
						callbacks.m_OnEvent(event, gamepad);
					}
				}
			}
			else if (event.gdevice.type == SDL_EVENT_GAMEPAD_ADDED)
			{
				Gamepad gamepad(event.gdevice.which);
				gamepad.Connect();
				m_Gamepads.emplace(event.gdevice.which, std::make_shared<DeviceController<Gamepad>>(std::move(gamepad)));
			}
			else if (event.gdevice.type == SDL_EVENT_GAMEPAD_REMOVED)
			{
				if (auto gamepadIter = m_Gamepads.find(event.gbutton.which); gamepadIter != m_Gamepads.end())
				{
					Gamepad& gamepad = gamepadIter->second->m_Device;
					auto& listeners = gamepadIter->second->m_Listeners;
					for (auto& [id, callbacks] : listeners) {
						callbacks.m_OnDisconnect(event, gamepad);
					}

					gamepad.Disconnect();
					m_Gamepads.erase(event.gbutton.which);
				}
			}
		}

		void ReloadDevices()
		{
			m_NextListenerId = 0;

			m_Gamepads.clear();
			m_Keyboard.reset();
			m_Mouse.reset();

			const auto gamepads = Input::Gamepad::GetGamepads();
			for (const auto id : gamepads)
			{
				Gamepad gamepad(id);
				gamepad.Connect();
				m_Gamepads.emplace(id, std::make_shared<DeviceController<Gamepad>>(std::move(gamepad)));
			}

			if (Input::Keyboard::IsConnected())
			{
				m_Keyboard = std::make_shared<DeviceController<Keyboard>>(Keyboard());
			}

			if (Input::Mouse::IsConnected())
			{
				m_Mouse = std::make_shared<DeviceController<Mouse>>(Mouse());
			}
		}

		std::vector<std::string> GetGamepadGUIDS() const 
		{
			std::vector<std::string> guids;
			guids.reserve(m_Gamepads.size());
			for (const auto& [id, gamepad] : m_Gamepads)
			{
				guids.emplace_back(gamepad->m_Device.GetSDLGUID());
			}
			return guids;
		}

	private:
		std::unordered_map<SDL_JoystickID, std::shared_ptr<DeviceController<Gamepad>>> m_Gamepads;
		std::shared_ptr<DeviceController<Keyboard>> m_Keyboard;
		std::shared_ptr<DeviceController<Mouse>> m_Mouse;
		DeviceListenerID m_NextListenerId = 0;
	};
}