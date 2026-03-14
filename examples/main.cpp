#include <iostream>
#include "aZeroInput.hpp"
#include "aZeroWindow.hpp"

#ifdef COMPILE_EXAMPLE
using namespace aZero;
using namespace Window;
using namespace Input;

class RenderWindow : public aZero::Window::Window_Win32 {
public:
	RenderWindow() = default;
	explicit RenderWindow(const aZero::Window::WindowDesc& desc, DeviceManager& deviceManager)
		:Window_Win32(desc), di_DeviceManager(&deviceManager)
	{
		m_KeyboardListener = deviceManager.ListenKeyboard({
			[this](const SDL_Event& event, Keyboard& keyboard){ 
				if (event.key.type == SDL_EVENT_KEY_DOWN) {
					if (event.key.key == SDLK_ESCAPE) {
						this->Close(); // Close window
					}
				}
				std::cout << "Event keyboard: ON\n"; 
			},
			[](const SDL_Event& event, Keyboard& keyboard){ std::cout << "Event keyboard: DISCONNECT\n"; }
			});

		for (const auto& guid : deviceManager.GetGamepadGUIDS())
		{
			m_GamepadListener = deviceManager.ListenGamepad(guid, 
				{
				[](const SDL_Event& event, Gamepad& gamepad) { std::cout << "Event gamepad: ON\n"; },
				[](const SDL_Event& event, Gamepad& gamepad) { std::cout << "Event gamepad: DISCONNECT\n"; }
				});
		}
	}

	GamepadListener m_GamepadListener;
	KeyboardListener m_KeyboardListener;

private:
	DeviceManager* di_DeviceManager;

	virtual void PollEventImpl(const SDL_Event& event) override {
		di_DeviceManager->ProcessEvent(event);
		if (event.gdevice.type == SDL_EVENT_GAMEPAD_ADDED)
		{
			const auto& guid = di_DeviceManager->GetGamepadGUIDS();
			m_GamepadListener = di_DeviceManager->ListenGamepad(guid[0],
				{
				[](const SDL_Event& event, Gamepad& gamepad) { 
					if(event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH && event.gbutton.down)
						std::cout << "Event gamepad: Pressed X\n";
				},
				[](const SDL_Event& event, Gamepad& gamepad) { std::cout << "Event gamepad: DISCONNECT\n"; }
				});
		}
	}
};

// API EXAMPLE
int main(int argc, char* argv[]) {
	aZero::Input::Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

	DeviceManager deviceManager;
	deviceManager.ReloadDevices();

	RenderWindow window(WindowDesc("MyWindow", { 0,0,800,600 }, { 255,255,0,0 }, SDL_WINDOW_RESIZABLE), deviceManager);

	const HWND handle = window.GetNativeHandle(); // Use for swapchain creation
	
	const std::vector<std::string> connectedDevices = deviceManager.GetGamepadGUIDS();
	for (const auto& guid : connectedDevices)
	{
		std::cout << "Device: " << guid << "\n";
	}

	KeyboardListener listener = deviceManager.ListenKeyboard({
	[](const SDL_Event& event, Keyboard& keyboard) {
		std::cout << "Main local keyboard: ON\n";
	},
	[](const SDL_Event& event, Keyboard& keyboard) { std::cout << "Main local keyboard: DISCONNECT\n"; }
		});

	while (window.IsOpen()) {
		deviceManager.UpdateDeviceStates();
		window.PollEvents();

		if (window.m_GamepadListener.IsValid()) {
			// TODO: Fix it. Currently not stopping if let go
			if (window.m_GamepadListener.GetDevice()->IsButtonDown(SDL_GAMEPAD_BUTTON_SOUTH))
				std::cout << "Event gamepad: X is down\n";
		}

		if (window.m_KeyboardListener.IsValid()) {
			if (window.m_KeyboardListener.GetDevice()->IsKeyDown(SDL_SCANCODE_A))
				std::cout << "Event keyboard: A is down\n";
		}
	}

	aZero::Input::Shutdown();

	return 0;
}
#endif // COMPILE_INPUT_EXAMPLE