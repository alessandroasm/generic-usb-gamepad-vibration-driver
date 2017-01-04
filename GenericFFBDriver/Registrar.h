
#if !defined(_REGISTRAR_H)
#define _REGISTRAR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "stdio.h"

class CRegistrar
{
protected:
	CRegistrar() {};

	BOOL SetInRegistry(HKEY hRootKey, LPCSTR subKey, LPCSTR keyName,LPCSTR keyValue)
	{
		HKEY hKeyResult;
		DWORD dataLength;
		DWORD dwDisposition;
		if (RegCreateKeyExA( hRootKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_WRITE, NULL, &hKeyResult, &dwDisposition) != ERROR_SUCCESS)
		{
			return FALSE;
		}
		dataLength = strlen(keyValue);
		DWORD retVal = RegSetValueExA( hKeyResult, keyName, 0, REG_SZ,(const BYTE *) keyValue, dataLength);
		RegCloseKey(hKeyResult);
		return (retVal == ERROR_SUCCESS) ? TRUE:FALSE;
	}

	BOOL DelFromRegistry(HKEY hRootKey, LPCSTR subKey)
	{
		long retCode;
		retCode = RegDeleteKeyA(hRootKey, subKey);
		if (retCode != ERROR_SUCCESS)
			return false;
		return true;
	}

	bool StrFromCLSID(REFIID riid,LPSTR strCLSID)
	{
		LPOLESTR pOleStr = NULL;
		HRESULT hr = ::StringFromCLSID(riid,&pOleStr);
		if(FAILED(hr))
			return false;
		int bytesConv = ::WideCharToMultiByte(CP_ACP,0,pOleStr,wcslen(pOleStr),strCLSID,MAX_PATH,NULL,NULL);
		CoTaskMemFree(pOleStr);
		strCLSID [ bytesConv ] = '\0';
		if(!bytesConv)
		{
			return false;
		}
		return true;
	}
public:
	bool RegisterObject(REFIID riid,LPCSTR LibId,LPCSTR ClassId)
	{
		char strCLSID [ MAX_PATH ];
		char Buffer [ MAX_PATH ];
		
		if(!strlen(ClassId))
			return false;

		if(!StrFromCLSID(riid,strCLSID))
			return false;
		
		if(!strlen(LibId) && strlen(ClassId))
			sprintf(Buffer,"%s.%s\\CLSID",ClassId,ClassId);
		else
			sprintf(Buffer,"%s.%s\\CLSID",LibId,ClassId);

		BOOL result;
		result = SetInRegistry(HKEY_CLASSES_ROOT,Buffer,"",strCLSID);
		if(!result)
			return false;
		sprintf(Buffer,"CLSID\\%s",strCLSID);
		char Class [ MAX_PATH ];
		sprintf(Class,"%s Class",ClassId);
		if(!SetInRegistry(HKEY_CLASSES_ROOT,Buffer,"",Class))
			return false;
		sprintf(Class,"%s.%s",LibId,ClassId);
		strcat(Buffer,"\\ProgId");

		return SetInRegistry(HKEY_CLASSES_ROOT,Buffer,"",Class) ? true:false;
	}

	bool UnRegisterObject(REFIID riid,LPCSTR LibId,LPCSTR ClassId)
	{
		char strCLSID [ MAX_PATH ];
		char Buffer [ MAX_PATH ];
		if(!StrFromCLSID(riid,strCLSID))
			return false;
		sprintf(Buffer,"%s.%s\\CLSID",LibId,ClassId);
		if(!DelFromRegistry(HKEY_CLASSES_ROOT,Buffer))
			return false;
		sprintf(Buffer,"%s.%s",LibId,ClassId);
		if(!DelFromRegistry(HKEY_CLASSES_ROOT,Buffer))
			return false;
		sprintf(Buffer,"CLSID\\%s\\ProgId",strCLSID);
		if(!DelFromRegistry(HKEY_CLASSES_ROOT,Buffer))
			return false;
		sprintf(Buffer,"CLSID\\%s",strCLSID);
		return DelFromRegistry(HKEY_CLASSES_ROOT,Buffer) ? true:false;
	}
};

class CDllRegistrar : public CRegistrar
{
public:
	bool RegisterObject(REFIID riid,LPCSTR LibId,LPCSTR ClassId,LPCSTR Path)
	{
		if(! CRegistrar::RegisterObject(riid,LibId,ClassId))
			return false;

		char strCLSID [ MAX_PATH ];
		char Buffer [ MAX_PATH ];
		if(!StrFromCLSID(riid,strCLSID))
			return false;
		
		sprintf(Buffer,"CLSID\\%s\\InProcServer32",strCLSID);
		if(! SetInRegistry(HKEY_CLASSES_ROOT,Buffer,"",Path))
			return false;

		return SetInRegistry(HKEY_CLASSES_ROOT, Buffer, "ThreadingModel", "Both") ? true : false;
	}

	bool UnRegisterObject(REFIID riid,LPCSTR LibId,LPCSTR ClassId)
	{
		char strCLSID [ MAX_PATH ];
		char Buffer [ MAX_PATH ];
		if(!StrFromCLSID(riid,strCLSID))
			return false;
		sprintf(Buffer,"CLSID\\%s\\InProcServer32",strCLSID);
		if(!DelFromRegistry(HKEY_CLASSES_ROOT,Buffer))
			return false;
		return CRegistrar::UnRegisterObject(riid,LibId,ClassId);
	}

};

#endif