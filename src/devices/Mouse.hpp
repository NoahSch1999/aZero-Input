#pragma once
#include <vector>
#include <array>
#include <type_traits>
#include <string>
#include <stdexcept>
#include "SDL3/SDL.h"

namespace aZero::Input {

	// NOTE: I dont like this class being pretty much a namespace since we cant have multiple mice. 
	// But its easier to have them under a single "namespace" than using the raw C function, so this is still ok...
	class Mouse {
	public:
		enum BUTTON { LEFT = 1, MIDDLE = 2, RIGHT = 3, X1 = 4, X2 = 5 };

		Mouse(const Mouse&) = delete;
		Mouse& operator=(const Mouse&) = delete;

		Mouse() = default;

		virtual ~Mouse() = default;

		Mouse(Mouse&& other) noexcept = default;

		Mouse& operator=(Mouse&& other) noexcept = default;

		static bool IsButtonDown(BUTTON button) {
			const auto buttonMask = SDL_BUTTON_MASK(button);
			return m_ButtonStates & buttonMask;
		}

		// TODO: Look into this. We might have to do the delta calulations manually using captured or global coordinates since this seems to be inconsistent...
		static void UpdateState() {
			m_ButtonStates = SDL_GetRelativeMouseState(&m_DeltaCoordinates[0], &m_DeltaCoordinates[1]);
		}

		static std::array<float, 2> GetDeltaCoordinates() {
			return m_DeltaCoordinates;
		}

		static std::array<float, 2> GetCapturedCoordinates() {
			std::array<float, 2> coordinates;
			SDL_GetMouseState(&coordinates[0], &coordinates[1]);
			return coordinates;
		}

		static std::array<float, 2> GetGlobalCoordinates() {
			std::array<float, 2> coordinates;
			SDL_GetGlobalMouseState(&coordinates[0], &coordinates[1]);
			return coordinates;
		}

		static SDL_Window* GetFocusedWindow() { return SDL_GetMouseFocus(); }

		// https://wiki.libsdl.org/SDL3/SDL_SetWindowRelativeMouseMode
		static bool EnableCapture(SDL_Window* window, bool enabled) {
			return SDL_SetWindowRelativeMouseMode(window, enabled);
		}

		static bool IsCaptureEnabled(SDL_Window* window) { return SDL_GetWindowRelativeMouseMode(window); }

		static bool IsCursorVisible() { return SDL_CursorVisible(); }

		static bool Show() { return SDL_ShowCursor(); }

		static bool Hide() { return SDL_HideCursor(); }

		static bool SetCursor(SDL_Cursor* cursor) { return SDL_SetCursor(cursor); }

		static bool SetDefaultCursor() {
			SDL_Cursor* cursor = SDL_GetDefaultCursor();
			if (cursor) {
				Mouse::SetCursor(cursor);
			}
			return cursor != NULL;
		}

		static bool SetSystemCursor(SDL_SystemCursor id) { 
			SDL_Cursor* cursor = SDL_CreateSystemCursor(id);
			if (cursor) {
				Mouse::SetCursor(cursor);
			}
			return cursor != NULL;
		}

		static SDL_Cursor* GetActiveCursor() { return SDL_GetCursor(); }

		static bool SetLocalBoundary(SDL_Window* window, const SDL_Rect* rect) {
			return SDL_SetWindowMouseRect(window, rect);
		}

		static const SDL_Rect* GetLocalBoundary(SDL_Window* window) {
			return SDL_GetWindowMouseRect(window);
		}

		static bool IsConnected() { return SDL_HasMouse(); }

		static std::vector<SDL_MouseID> GetMice() {
			int32_t count = 0;
			SDL_MouseID* ids = SDL_GetMice(&count);

			std::vector<SDL_MouseID> mice;
			mice.reserve(count);
			for (int32_t i = 0; i < count; i++) {
				mice.emplace_back(ids[i]);
			}
			SDL_free(ids);

			return mice;
		}

		static std::vector<std::string> GetMiceNames() {
			const std::vector<SDL_MouseID> mice = Mouse::GetMice();

			std::vector<std::string> names;
			names.reserve(mice.size());
			for (SDL_MouseID id : mice) {
				if (id == 0)
					continue;
				names.emplace_back(SDL_GetMouseNameForID(id));
			}
			return names;
		}

		static std::string GetButtonName(BUTTON button) { return m_ButtonNames[button - 1]; }

		// TODO: Implement cursor image loading + setting
		// https://wiki.libsdl.org/SDL3/SDL_CreateCursor
		// https://wiki.libsdl.org/SDL3/SDL_CreateColorCursor
		// https://wiki.libsdl.org/SDL3/SDL_CreateSystemCursor
		// https://wiki.libsdl.org/SDL3/SDL_CreateAnimatedCursor
		// https://wiki.libsdl.org/SDL3/SDL_DestroyCursor
		
		// TODO: Implement grabbing stuff?
		// SDL_SetWindowMouseGrab

	private:
		inline static std::array<std::string, 5> m_ButtonNames = { "LEFT", "MIDDLE", "RIGHT", "X1", "X2" };
		inline static std::array<float, 2> m_DeltaCoordinates;
		inline static SDL_MouseButtonFlags m_ButtonStates;
	};
}
