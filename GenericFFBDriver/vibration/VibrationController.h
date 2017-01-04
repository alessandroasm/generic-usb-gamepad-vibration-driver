#pragma once
#include "../stdafx.h"
#include <string>
#include <mutex>

namespace vibration {

	class VibrationController
	{
		static std::wstring hidDevPath;
		static std::mutex mtxSync;
		static std::unique_ptr<std::thread> thrVibration;

		VibrationController();
		~VibrationController();

		static void StartVibrationThread();
		static void VibrationThreadEntryPoint();

	public:
		static void SetHidDevicePath(LPWSTR path);
		static void StartEffect(DWORD dwEffectID, LPCDIEFFECT peff);
		static void StopEffect(DWORD dwEffectID);
		static void StopAllEffects();
		static void Reset();
	};

}

