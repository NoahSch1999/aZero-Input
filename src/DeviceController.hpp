#pragma once
#include <unordered_map>

#include "DeviceListener.hpp"

namespace aZero::Input {
	template<typename DeviceType>
	struct DeviceController
	{
		friend class DeviceListener<DeviceType>;
		friend class DeviceManager;
	public:

		DeviceController() = default;

		DeviceController(DeviceType&& device)
			:m_Device(std::move(device))
		{

		}

	private:
		DeviceType m_Device;
		std::unordered_map<DeviceListenerID, ListenerCallbacks<DeviceType>> m_Listeners;

		void StopListen(DeviceListener<DeviceType>& listener)
		{
			if (listener.IsValid())
			{
				m_Listeners.erase(listener.m_Id);
			}
		}
	};

	template<typename DeviceType>
	aZero::Input::DeviceListener<DeviceType>::~DeviceListener<DeviceType>()
	{
		if (!m_Controller.expired())
		{
			auto shared = m_Controller.lock();
			shared->StopListen(*this);
		}
	}
}
