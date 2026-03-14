#pragma once
#include <functional>
#include <string>
#include <limits>
#include <memory>

#include "SDL3/SDL.h"

namespace aZero::Input {
	// Forward declarations
	class DeviceManager;

	template<typename DeviceType>
	class DeviceController;

	// Aliases
	using DeviceListenerID = uint32_t;

	template<typename DeviceType>
	using ListenerCallback = std::function<void(const SDL_Event&, DeviceType&)>;

	template<typename DeviceType>
	struct ListenerCallbacks
	{
		ListenerCallback<DeviceType> m_OnEvent;
		ListenerCallback<DeviceType> m_OnDisconnect;
		ListenerCallbacks() = default;
		ListenerCallbacks(ListenerCallback<DeviceType>&& onEvent, ListenerCallback<DeviceType>&& onDisconnect)
			:m_OnEvent(std::move(onEvent)), m_OnDisconnect(std::move(onDisconnect)) {
		}
	};

	// Globals
	static constexpr DeviceListenerID INVALID_LISTENING_ID = std::numeric_limits<DeviceListenerID>::max();

	// Class definitions
	template<typename DeviceType>
	struct DeviceListener
	{
		friend class DeviceManager;
		friend class DeviceController<DeviceType>;
	public:
		DeviceListener(const DeviceListener&) = delete;
		DeviceListener& operator=(const DeviceListener&) = delete;

		DeviceListener() = default;
		DeviceListener(std::weak_ptr<DeviceController<DeviceType>> manager, DeviceType& device, DeviceListenerID id, const std::string& guid)
			:m_Controller(manager), m_Device(&device), m_Id(id), m_Guid(guid) {
		}

		DeviceListener(DeviceListener&& other) noexcept
		{
			*this = std::move(other);
		}

		DeviceListener& operator=(DeviceListener&& other) noexcept
		{
			std::swap(m_Controller, other.m_Controller);
			std::swap(m_Device, other.m_Device);
			std::swap(m_Guid, other.m_Guid);
			std::swap(m_Id, other.m_Id);
			return *this;
		}

		// Forward declare since we need to use DeviceController<DeviceType> in the destructor
		~DeviceListener();

		bool IsValid() const
		{
			return !m_Controller.expired();
		}

		std::string GetDeviceGUID() const { return m_Guid; }
		DeviceType* GetDevice() const { return m_Device; }

	private:
		DeviceListenerID m_Id = INVALID_LISTENING_ID;
		DeviceType* m_Device = nullptr;
		std::string m_Guid;
		std::weak_ptr<DeviceController<DeviceType>> m_Controller;

		void Invalidate()
		{
			m_Device = nullptr;
			m_Controller = std::weak_ptr<DeviceController<DeviceType>>();
			m_Id = INVALID_LISTENING_ID;
			m_Guid = "";
		}
	};
}
