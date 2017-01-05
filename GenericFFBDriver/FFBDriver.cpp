
#include "FFBDriver.h"
#include "vibration/VibrationController.h"
#include <fstream>

void LogMessage(const char* msg) {

	SYSTEMTIME st;
	GetSystemTime(&st);
	char buffer[256];

	sprintf_s(buffer, "[ %04d-%02d-%02d %02d:%02d:%02d.%03d ] %s",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond,
		st.wMilliseconds, msg);

	printf(buffer);

	std::ofstream outfile;
	outfile.open("J:\\Gamepad\\driverlog.txt", std::ios_base::app);
	outfile << buffer;
	outfile.close();
}

FFBDriver::FFBDriver()
{
}

FFBDriver::~FFBDriver()
{
}


STDMETHODIMP FFBDriver::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputEffectDriver))
	{
		*ppv = (IDirectInputEffectDriver *)this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}


HRESULT STDMETHODCALLTYPE FFBDriver::DeviceID(
		DWORD             dwDIVer,
		DWORD             dwExternalID,
		DWORD             fBegin,
		DWORD             dwInternalId,
		LPVOID            lpInfo) 
{
	LPDIHIDFFINITINFO lpDIHIDInitInfo = (LPDIHIDFFINITINFO)lpInfo;

#ifdef _DEBUG
	char buff[100];
	sprintf_s(buff, "DeviceID\n\tdwDIVer=0x%04x\n\tdwExternalID=0x%04x\n\tfBegin=0x%04x\n\tdwInternalId=0x%04x\n\n",
		dwDIVer, dwExternalID, fBegin, dwInternalId);
	LogMessage(buff);
#endif

	vibration::VibrationController::SetHidDevicePath(lpDIHIDInitInfo->pwszDeviceInterface);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE FFBDriver::GetVersions(LPDIDRIVERVERSIONS lpVersions) {
	LogMessage("GetVersions\n");

	lpVersions->dwFFDriverVersion = 0x100;
	lpVersions->dwFirmwareRevision = 0x100;
	lpVersions->dwHardwareRevision = 0x100;

	return S_OK;
}
HRESULT STDMETHODCALLTYPE FFBDriver::Escape(THIS_ DWORD, DWORD, LPDIEFFESCAPE) {
	LogMessage("Escape!\n");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE FFBDriver::SetGain(
	DWORD dwID,
	DWORD dwGain) 
{
#ifdef _DEBUG
	char buff[100];
	sprintf_s(buff, "SetGain\n\tdwID=0x%04x\n\tdwGain=0x%04x\n\n",
		dwID, dwGain);
	LogMessage(buff);
#endif

	/*
	DWORD newGain = (dwGain / 38);
	if (newGain > 0xfe)
		currentGain = 0xfe;
	else
		currentGain = (byte)newGain;
	*/

	return S_OK;
}

HRESULT STDMETHODCALLTYPE FFBDriver::SendForceFeedbackCommand(
	DWORD dwID,
	DWORD dwCommand) 
{
#ifdef _DEBUG
	char buff[100];
	sprintf_s(buff, "SendForceFeedbackCommand\n\tdwID=0x%04x\n\tdwCommand=0x%04x\n\n",
		dwID, dwCommand);
	LogMessage(buff);
#endif

	switch (dwCommand) {
	case DISFFC_RESET:
		vibration::VibrationController::Reset();
		break;

	case DISFFC_STOPALL:
		vibration::VibrationController::StopAllEffects();
		break;

	case DISFFC_PAUSE:
	case DISFFC_CONTINUE:
	case DISFFC_SETACTUATORSON:
	case DISFFC_SETACTUATORSOFF:
		break;
	}
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FFBDriver::GetForceFeedbackState(THIS_ DWORD, LPDIDEVICESTATE) {
	LogMessage("GetForceFeedbackState!\n");
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FFBDriver::DownloadEffect(
	DWORD       dwID,
	DWORD       dwEffectID,
	LPDWORD     pdwEffect,
	LPCDIEFFECT peff,
	DWORD       dwFlags) 
{
#ifdef _DEBUG
	char buff[100];
	sprintf_s(buff, "DownloadEffect\n\tdwID=0x%04x\n\tdwEffectID=0x%04x\n\tdwFlags=0x%04x\n\n",
		dwID, dwEffectID, dwFlags);

	LogMessage(buff);
#endif

	vibration::VibrationController::StartEffect(dwEffectID, peff);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE FFBDriver::DestroyEffect(DWORD, DWORD) {
	LogMessage("DestroyEffect!\n");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE FFBDriver::StartEffect(DWORD, DWORD, DWORD, DWORD) {
	LogMessage("StartEffect!\n");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE FFBDriver::StopEffect(DWORD dwID, DWORD dwEffect) {
	LogMessage("StopEffect!\n");
	return S_OK;
}
HRESULT STDMETHODCALLTYPE FFBDriver::GetEffectStatus(DWORD, DWORD, LPDWORD) {
	LogMessage("GetEffectStatus!\n");
	return S_OK;
}
