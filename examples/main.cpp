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
		// Setup escape key as application exit
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

		if (deviceManager.GetGamepadGUIDS().size() > 0) {
			m_GamepadListener = deviceManager.ListenGamepad(deviceManager.GetGamepadGUIDS()[0],
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
		di_DeviceManager->ProcessEvent(event); // Check if theres any events regarding devices and handle it
	}
};

// API EXAMPLE
int main(int argc, char* argv[]) {
	aZero::Input::Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

	// Create and load the device manager and connected devices
	DeviceManager deviceManager;
	deviceManager.ReloadDevices();

	RenderWindow window(WindowDesc("MyWindow", { 0,0,1920,1080 }, { 255,255,0,0 }, SDL_WINDOW_RESIZABLE), deviceManager);

	const HWND handle = window.GetNativeHandle(); // Use for swapchain creation
	
	// Print guids of all connected gamepads
	const std::vector<std::string> connectedDevices = deviceManager.GetGamepadGUIDS();
	for (const auto& guid : connectedDevices)
	{
		std::cout << "Device: " << guid << "\n";
	}

	// Setup listener on the keyboard
	KeyboardListener listener = deviceManager.ListenKeyboard({
	[](const SDL_Event& event, Keyboard& keyboard) {
		std::cout << "Main local keyboard: ON\n";
	},
	[](const SDL_Event& event, Keyboard& keyboard) { std::cout << "Main local keyboard: DISCONNECT\n"; }
		});

	// Setup listener on the mouse
	MouseListener mouseListener = deviceManager.ListenMouse({
	[](const SDL_Event& event, Mouse& mouse) {
			if (event.mdevice.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				std::cout << Mouse::GetButtonName((Mouse::BUTTON)event.button.button) << "\n"; // Print button name
			}
			if (event.mdevice.type == SDL_EVENT_MOUSE_MOTION) {
				auto delta = Mouse::GetCapturedCoordinates();
				std::cout << "x delta: " << delta[0] << "\n";
				std::cout << "y delta: " << delta[1] << "\n";
			}
	},
	[](const SDL_Event& event, Mouse& mouse) { 
			std::cout << "Main local mouse: DISCONNECT\n"; 
		}
		});

	while (window.IsOpen()) {
		deviceManager.UpdateDeviceStates(); // Query keyboard and mouse states
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

		if (Mouse::IsButtonDown(Mouse::MIDDLE))
		{
			std::cout << "Middle mouse button is down\n";
		}
	}

	aZero::Input::Shutdown();

	return 0;
}
#endif // COMPILE_INPUT_EXAMPLE