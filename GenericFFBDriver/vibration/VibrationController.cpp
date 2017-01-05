#include "VibrationController.h"
#include <algorithm>
#include <Hidsdi.h>

#define MAX_EFFECTS 5
#define MAXC(a, b) ((a) > (b) ? (a) : (b))

namespace vibration {
	bool quitVibrationThread = false;

	const byte GP_STOP_COMMAND[8] = {
		0x00, 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	void SendHidCommand(HANDLE hHidDevice, const byte* buff, DWORD buffsz) {
		HidD_SetOutputReport(hHidDevice, (PVOID)buff, 8);
	}
	void SendVibrationForce(HANDLE hHidDevice, byte forceSmallMotor, byte forceBigMotor) {
		const byte buffer1[8] = {
			0x00, 0x51, 0x00, forceSmallMotor, 0x00, forceBigMotor, 0x00, 0x00
		};
		const byte buffer2[8] = {
			0x00, 0xfa, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		SendHidCommand(hHidDevice, (const byte*)&buffer1, 8);
		SendHidCommand(hHidDevice, (const byte*)&buffer2, 8);
	}
	void SendVibrationStop(HANDLE hHidDevice) {
		SendHidCommand(hHidDevice, (const byte*)&GP_STOP_COMMAND, 8);
	}

	struct VibrationEff {
		DWORD dwEffectId;
		DWORD dwStartFrame;
		DWORD dwStopFrame;

		byte forceX;
		byte forceY;

		BOOL isActive;
		BOOL started;
	};

	VibrationEff VibEffects[MAX_EFFECTS];

	std::wstring VibrationController::hidDevPath;
	std::mutex VibrationController::mtxSync;
	std::unique_ptr<std::thread, VibrationController::VibrationThreadDeleter> VibrationController::thrVibration;

	VibrationController::VibrationController()
	{
	}


	VibrationController::~VibrationController()
	{
	}

	void VibrationController::StartVibrationThread()
	{
		mtxSync.lock();

		if (thrVibration == NULL) {
			quitVibrationThread = false;

			for (int k = 0; k < MAX_EFFECTS; k++)
				VibEffects[k].isActive = FALSE;

			thrVibration.reset(new std::thread(VibrationController::VibrationThreadEntryPoint));
		}

		mtxSync.unlock();
	}

	void VibrationController::VibrationThreadEntryPoint()
	{
		// Initialization
		HANDLE hHidDevice = CreateFile(
			hidDevPath.c_str(),
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		byte lastForceX = 0;
		byte lastForceY = 0;

		while (true) {
			mtxSync.lock();

			if (quitVibrationThread) {
				mtxSync.unlock();
				break;
			}

			DWORD frame = GetTickCount();
			byte forceX = 0;
			byte forceY = 0;

			for (int k = 0; k < MAX_EFFECTS; k++) {
				if (!VibEffects[k].isActive)
					continue;

				if (VibEffects[k].started) {

					if (VibEffects[k].dwStopFrame != INFINITE) {

						if (VibEffects[k].dwStopFrame <= frame) {
							VibEffects[k].isActive = FALSE;

						}
						else {
							forceX = MAXC(forceX, VibEffects[k].forceX);
							forceY = MAXC(forceY, VibEffects[k].forceY);

						}

					}
					else {
						forceX = MAXC(forceX, VibEffects[k].forceX);
						forceY = MAXC(forceY, VibEffects[k].forceY);
					}
				}
				else {
					if (VibEffects[k].dwStartFrame <= frame) {
						VibEffects[k].started = TRUE;
						
						if (VibEffects[k].dwStopFrame != INFINITE) {
							DWORD dt = VibEffects[k].dwStopFrame - VibEffects[k].dwStartFrame;
							VibEffects[k].dwStopFrame = frame + dt;
						}

						forceX = MAXC(forceX, VibEffects[k].forceX);
						forceY = MAXC(forceY, VibEffects[k].forceY);
					}
				}
			}

			if (forceX != lastForceX || forceY != lastForceY) {
				// Send the command
				if (forceX == 0 && forceY == 0)
					SendVibrationStop(hHidDevice);
				else
					SendVibrationForce(hHidDevice, forceX, forceY);

				lastForceX = forceX;
				lastForceY = forceY;
			}

			mtxSync.unlock();

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (hHidDevice != NULL) {
			// Send stop command
			SendVibrationStop(hHidDevice);

			CloseHandle(hHidDevice);
		}
	}

	void VibrationController::SetHidDevicePath(LPWSTR path)
	{
		hidDevPath = path;
		Reset();
	}

	void VibrationController::StartEffect(DWORD dwEffectID, LPCDIEFFECT peff)
	{
		mtxSync.lock();
		for (int k = 0; k < MAX_EFFECTS; k++) {
			if (VibEffects[k].isActive && k < MAX_EFFECTS - 1)
				continue;

			// Calculating intensity
			byte forceX = 0xfe;
			byte forceY = 0xfe;

			if (peff->cAxes >= 1) {
				LONG fx = peff->rglDirection[0];
				if (fx <= 1) fx = peff->dwGain;

				forceX = forceY = (byte)abs(round((((double)fx) / 10000.0) * 254.0));
			}

			if (peff->cAxes >= 2) {
				LONG fy = peff->rglDirection[1];
				if (fy <= 1) fy = peff->dwGain;

				forceY = (byte)abs(round((((double)fy) / 10000.0) * 254.0));
			}


			DWORD frame = GetTickCount();

			VibEffects[k].forceX = forceX;
			VibEffects[k].forceY = forceY;

			VibEffects[k].dwEffectId = dwEffectID;
			VibEffects[k].dwStartFrame = frame + (peff->dwStartDelay / 1000);
			VibEffects[k].dwStopFrame = 
				peff->dwDuration == INFINITE ? INFINITE : 
				VibEffects[k].dwStartFrame + (peff->dwDuration / 1000);
			VibEffects[k].isActive = TRUE;
			VibEffects[k].started = FALSE;

			break;
		}

		mtxSync.unlock();

		StartVibrationThread();
	}

	void VibrationController::StopEffect(DWORD dwEffectID)
	{
		mtxSync.lock();
		for (int k = 0; k < MAX_EFFECTS; k++) {
			if (VibEffects[k].dwEffectId != dwEffectID)
				continue;

			VibEffects[k].dwStopFrame = 0;
		}
		
		mtxSync.unlock();
	}

	void VibrationController::StopAllEffects()
	{
		mtxSync.lock();
		for (int k = 0; k < MAX_EFFECTS; k++) {
			VibEffects[k].dwStopFrame = 0;
		}
		mtxSync.unlock();

		Reset();
	}

	void VibrationController::Reset()
	{
		if (thrVibration == NULL)
			return;

		mtxSync.lock();
		quitVibrationThread = true;
		mtxSync.unlock();

		thrVibration->join();
		thrVibration.reset(NULL);
	}

}
