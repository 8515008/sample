// rrstrigger.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "rrstrigger.h"
#include <vector>  // For vector template
#include <iostream>
#include <string>
#include "Evntprov.h"
#include "publisher.h"  // Generated from manifest
#include <string.h>
#include <strsafe.h>
#include <tlhelp32.h>
#include <Wtsapi32.h>
#include <Shellapi.h>
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "shell32.lib")

#pragma comment(lib, "User32.lib")


#define MAX_LOADSTRING 100
// Mutexes
#define RR_SERVICE_MUTEX			_T("Global\\RR Service")
#define SCHEDULER_PROXY_EXE         _T("scheduler_proxy.exe")
#define	TVTSETSCHED_EXE			_T("tvtsetsched.exe")

#define SUNDAY     0X1
#define MONDAY     0X2
#define TUESDAY    0X4
#define WEDNESDAY  0X8
#define THURSDAY   0X10
#define FRIDAY     0X20
#define SATURDAY   0X40


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
//

enum TRANSFER_TYPE {
  Download = 1,
  Upload,
  UploadReply
};

#define MAX_NAMEDVALUES 5  // Maximum array size

// Name/value pair
typedef struct _namedvalue {
  LPWSTR name;
  USHORT value;
} NAMEDVALUE, *PNAMEDVALUE;

//  DWORD status = ERROR_SUCCESS;
  std::vector<EVENT_DATA_DESCRIPTOR> Descriptors;  // Array of data descriptors
  EVENT_DATA_DESCRIPTOR EventData;  // Temp data descriptor for loading vector

  //// {B52C6D34-59C9-4DFC-80CB-3B6502F5D5CC}
  //IMPLEMENT_OLECREATE(<< class >> , << external_name >> ,
  //0xb52c6d34, 0x59c9, 0x4dfc, 0x80, 0xcb, 0x3b, 0x65, 0x2, 0xf5, 0xd5, 0xcc
	 // );

  GUID PUBLISHER_GUID = { 0xb52c6d34, 0x59c9, 0x4dfc, { 0x80, 0xcb, 0x3b, 0x65, 0x2, 0xf5, 0xd5, 0xcc } };
  //GUID PUBLISHER_GUID = { 0x27b66b66, 0x6df9, 0x4e63, { 0xb6, 0xc0, 0xdd, 0x18, 0x81, 0xc8, 0xbc, 0xf9 } };
  REGHANDLE RegistrationHandle = NULL;    // Provider registration handle

  // Data to load into event descriptor
  DWORD TransferType = Upload;
  DWORD Day = MONDAY | TUESDAY;
  LPWSTR Path = L"c:\\path\\folder\\file.ext";
  BYTE Cert[11] = {0x2, 0x4, 0x8, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x0, 0x1};
  PBYTE Guid = (PBYTE) &PUBLISHER_GUID;
  DWORD BufferSize = sizeof(GUID);
  USHORT ArraySize = MAX_NAMEDVALUES;
  BOOL IsLocal = TRUE;
  NAMEDVALUE NamedValues[MAX_NAMEDVALUES] = { 
    {L"Bill", 1},
    {L"Bob", 2},
    {L"William", 3},
    {L"Robert", 4},
    {L"", 5}
  };


 typedef ULONG (WINAPI *PEVENTREGISTER)(
				LPCGUID ProviderId,
				PENABLECALLBACK EnableCallback,
				PVOID CallbackContext,
				PREGHANDLE RegHandle
				);
 typedef ULONG (WINAPI *PEVENTUNREGISTER)(
					REGHANDLE RegHandle
				);
 typedef ULONG (WINAPI *PEVENTWRITE)(
				REGHANDLE RegHandle,
				PCEVENT_DESCRIPTOR EventDescriptor,
				ULONG UserDataCount,
				PEVENT_DATA_DESCRIPTOR UserData
				);
 PEVENTREGISTER pEventRegister = NULL;
 PEVENTUNREGISTER pEventUnregister = NULL;
 PEVENTWRITE pEventWrite = NULL;
 BOOL Init()
 {
    HMODULE hDll = LoadLibrary(L"Advapi32.dll");
	if (hDll != NULL) {
		pEventRegister = (PEVENTREGISTER)GetProcAddress(hDll, "EventRegister");
		pEventUnregister =(PEVENTUNREGISTER)GetProcAddress(hDll, "EventUnregister");
		pEventWrite = (PEVENTWRITE)GetProcAddress(hDll, "EventWrite");

		if(pEventRegister != NULL && pEventUnregister != NULL && pEventWrite != NULL)
		{
			FreeLibrary(hDll);
			return TRUE;
		}
		else
		{
			FreeLibrary(hDll);
			return FALSE;
		}
       
    }
    return FALSE;
 }

  void usage()
  {
	  printf("Lenovo service trigger v0.1.\n\n");
  }

  int startService()
  {
		
			DWORD status = ERROR_SUCCESS;

		  status = pEventRegister(
				 &PUBLISHER_GUID,    // GUID that identifies the provider
				 NULL,               // Callback not used
				 NULL,               // Context noot used
				 &RegistrationHandle // Used when calling UnregisterTraceGuids
				 );

		  if (ERROR_SUCCESS != status)
		  {
			 wprintf(L"EventRegister failed with 0x%x", status);
			TCHAR temp[1024] = {0};
			_swprintf(temp, L"EventRegister failed with 0x%x", status);
			wprintf(temp);
			return -1;
		  }
          else
          {
              _tprintf(_T("Event registered successfully\n"));
          }


		  // Load up the data descriptor vector for the GenericDataEvent event. 
		  // Add the data to the vector in the order of <data> elements
		  // defined in the template for the event. 
		   
		  EventDataDescCreate(&EventData, &BufferSize, sizeof(DWORD));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, Guid, sizeof(GUID));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, Cert, sizeof(Cert));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, &IsLocal, sizeof(BOOL));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, Path, (ULONG)(wcslen(Path) + 1) * sizeof(WCHAR));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, &ArraySize, sizeof(USHORT));
		  Descriptors.push_back( EventData );

		  // If your event contains a structure, you should write each member
		  // of the structure separately. If the structure contained integral data types
		  // such as DWORDs and the data types were aligned on an 8-byte boundary, you 
		  // could use the following call to write the structure, however, you are 
		  // encouraged to write the members separately.
		  //
		  // EventDataDescCreate(&EvtData, struct, sizeof(struct));
		  //
		  // Because the array of structures in this example contains both strings 
		  // and numbers, you must write each member of the structure, for each element of 
		  // the array element, separately.

		  for (int i = 0; i < MAX_NAMEDVALUES; i++)
		  {
			EventDataDescCreate( &EventData, NamedValues[i].name, (ULONG)(wcslen(NamedValues[i].name)+1) * sizeof(WCHAR) );
			Descriptors.push_back( EventData );

			EventDataDescCreate( &EventData, &(NamedValues[i].value), sizeof(USHORT) );
			Descriptors.push_back( EventData );
		  }

		  EventDataDescCreate(&EventData, &Day, sizeof(DWORD));
		  Descriptors.push_back( EventData );

		  EventDataDescCreate(&EventData, &TransferType, sizeof(DWORD));
		  Descriptors.push_back( EventData );
		  // Write the event. You do not have to verify if you provider is enabled before
		  // writing the event. ETW will write the event to any session that enabled
		  // the provider. If no session enabled the provider, the event is not 
		  // written. If you need to perform extra work to write an event that you
		  // would not otherwise do, you may want to call the EventEnabled function
		  // before performing the extra work. The EventEnabled function tells you if a
		  // session has enabled your provider so you know if you should perform the 
		  // extra work or not.

		  status = pEventWrite(
			  RegistrationHandle,       // From EventRegister
			  &GenericDataEvent,        // EVENT_DESCRIPTOR generated from the manifest
			  (ULONG)Descriptors.size(), // Size of the array of EVENT_DATA_DESCRIPTORs
			  &Descriptors[0]            // Array of descriptors that contain the event data
			  );

		  if (status != ERROR_SUCCESS) 
		  {
			wprintf(L"EventWrite failed with 0x%x", status);
			TCHAR temp[1024] = {0};
			_swprintf(temp, L"EventWrite failed with 0x%x", status);
			wprintf(temp);
			return -2;
		  }

        _tprintf(_T("Start event successfully raised\n"));

		return 0;
  }
  int QuoteString(const TCHAR * tpSrc, TCHAR * tpDes, unsigned short sizeOfDest)
  {
	  if(tpSrc == NULL || tpDes == NULL) return 3; //null pointer
	  size_t srcLen = _tcslen(tpSrc);	TCHAR * tempDes = tpDes;
	  if(tpSrc[0] == _T('"') && tpSrc[srcLen-1] == _T('"'))return 1; //already enclosed
	  if (sizeOfDest < (srcLen+3)) return 2; //no enough space reserved for target 
	  memset(tpDes,0,sizeOfDest);//initialize target
	  tpDes[0] = _T('"');	tempDes++;
	  while(*tpSrc)*tempDes++ = *tpSrc++;  tempDes[0]='"';
	  return 0; //successfully
  }
  void stopService()
  {
	if( RegistrationHandle )
	{
		if( ERROR_SUCCESS  == pEventUnregister(RegistrationHandle) )
		{
			wprintf(L"Unregister rrservice provider successfully!");
			RegistrationHandle = NULL;
		}else
		{
			TCHAR temp[1024] = {0};
			_swprintf(temp, L"Failed to Unregister rrservice provider! error code %ld", GetLastError());
			wprintf(temp);
			
		}
	}
  }


BOOL StartXPOrVistaService()
{
	BOOL bRet = FALSE;
	LPCTSTR lpServiceName = _T("TVT Backup Service");

	//Let's Open the SCM 
	SC_HANDLE scm = OpenSCManager(0,0,  SC_MANAGER_CONNECT);
	

	if (scm != NULL)
	{
		//Get the service's handle
		SC_HANDLE service = OpenService(scm, lpServiceName, SERVICE_QUERY_STATUS | SERVICE_START);

		if (service != NULL)
		{
			//Let's query the service status...
			//if service is running stop it
			SERVICE_STATUS status;
			BOOL bSuccess = QueryServiceStatus(service, &status);
			if (bSuccess)
			{
				if (status.dwCurrentState == SERVICE_STOPPED)
				{
					//Start the service
					if (StartService(service,0,0) == TRUE)
					{
						//MessageBox(NULL,_T("service start failed!"),_T("service start failed!"), 0 );
						bRet = TRUE;
					}
				}
				else if(status.dwCurrentState == SERVICE_RUNNING)
				{
					bRet = TRUE;
				}
			}
			CloseServiceHandle(service);
		}
		CloseServiceHandle(scm);
	}
	return bRet;
}

BOOL Start_scheduler_proxy_xp()
{
	// Check whether scheduler_proxy is running or not
	BOOL bRet = FALSE;
    BOOL bFound = FALSE;
    HANDLE  hProcessSnap = NULL;
    PROCESSENTRY32 pe32 = {0};
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE)
    {
        pe32.dwSize = sizeof(PROCESSENTRY32); 
        if(Process32First(hProcessSnap, &pe32)) 
        { 
            do 
            { 
                if(lstrcmpi(pe32.szExeFile , _T("scheduler_proxy.exe")) == 0)
                {
                    bFound = TRUE;
                    break;
                }
            }
            while (Process32Next(hProcessSnap, &pe32)); 
        }
    }
    else
    {
        return bRet;
    }

    if(hProcessSnap && hProcessSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hProcessSnap);
    }

    if (bFound)
    {
        return TRUE;
    }
	// start the process
	{
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		GetStartupInfo(&si);
		TCHAR szTargetExe[MAX_PATH] = {0};
		StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\%s"), _tgetenv(_T("CommonProgramFiles")), SCHEDULER_PROXY_EXE);
		
		// Added by daniel 2013-5-23 for fix vulnarability issue.
		TCHAR strCmdwithQuotes[MAX_PATH] = {0};
		int ret = QuoteString(szTargetExe,strCmdwithQuotes,MAX_PATH);
		ret = CreateProcess( NULL,
			strCmdwithQuotes,
			NULL,
			NULL,
			FALSE,
			CREATE_NO_WINDOW|DETACHED_PROCESS,
			NULL,
			NULL,
			&si,
			&pi );
		if (ret == 0)
		{
			ret = CreateProcess( NULL,
				szTargetExe,
				NULL,
				NULL,
				FALSE,
				CREATE_NO_WINDOW|DETACHED_PROCESS,
				NULL,
				NULL,
				&si,
				&pi );
		}
		if(ret != 0)
		{
			bRet = TRUE;
			if(pi.hThread != NULL)
				CloseHandle(pi.hThread);
			if(pi.hProcess != NULL)
				CloseHandle(pi.hProcess);
		}
	}

	return bRet;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	//
	int iResult = 0;

	// David Feng change here begin
	// Notes: this app for trigger the service, previous version is console application , so it will show the prompt window
	// now change it to win32 app, it will not show any windows anymore  2010/6/13

	int argc = 0;



	// GetCommandLineW return value is a pointer to the command-line string for the current process.
	// so it can call directly
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	WCHAR wAction[_MAX_PATH]= {0};
	
	if(argc < 2 || argv == NULL)
	{
		GlobalFree((HGLOBAL)argv);
		return 1;
	}

	StringCbCopy(wAction, sizeof(wAction), argv[1]);
	GlobalFree((HGLOBAL)argv);
		
   // usage();

    LPWSTR pAction = &wAction[0];

	// David Feng Change end

	// add by davidyf to support xp issue 2010-7-22

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	DWORD dwOSVersion = 0;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ( (OSVERSIONINFO *)&osvi) ) )
	{
	  osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	  if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		 return 1; // failed
	}

	dwOSVersion = osvi.dwMajorVersion * 100000 + osvi.dwMinorVersion * 10000 + osvi.dwBuildNumber;


	if( dwOSVersion < 617000 ) // not windows 7
	{
		if (0 == _wcsicmp(pAction, L"-start") )
		{
			Start_scheduler_proxy_xp(); // for xp, we will start the scheduler_proxy anyway, do not care if it failed.

			//::MessageBox(NULL, L"this is not win7", L"this is not win7", 0);
			if(StartXPOrVistaService() == TRUE )
				return 0;
			else
				return 1;
		}
		else
		{
			return 1;
		}
	}

	if(!Init())
		return 1;

	// add by davidyf end 2010-7-22


    if (0 == _wcsicmp(pAction, L"-start") )
    {
        startService();

        stopService();

        // Check whether rrservice.exe is running or not @gordenye_472332
        BOOL bFound = FALSE;
        HANDLE  hProcessSnap = NULL;
        PROCESSENTRY32 pe32 = {0};
        int i = 0;

        //Wait 4 minutes (60 * 4 seconds) for the service to start-up
        while (!bFound && i < 60)
        {
            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap != INVALID_HANDLE_VALUE)
            {
                pe32.dwSize = sizeof(PROCESSENTRY32); 
                if(Process32First(hProcessSnap, &pe32)) 
                { 
                    do 
                    { 
                        if(lstrcmpi(pe32.szExeFile , _T("rrservice.exe")) == 0)
                        {
                            bFound = TRUE;
                            break;
                        }
                    }
                    while (Process32Next(hProcessSnap, &pe32)); 
                }
            }

            if (!bFound)
                Sleep(4000);

            i++;
        }
        
        if(hProcessSnap && hProcessSnap != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hProcessSnap);
        }

        if (!bFound)
        {
            _tprintf(_T("Command line Startup: Service not found.\n"));
            return 2; // Service not found
        }
        else
        {
            _tprintf(_T("Command line Startup: Service is running.\n"));
        }
    }
    else
    {
        return 1;
    }

    // Check whether scheduler_proxy is running or not
    BOOL bFound = FALSE;
    HANDLE  hProcessSnap = NULL;
    PROCESSENTRY32 pe32 = {0};
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE)
    {
        pe32.dwSize = sizeof(PROCESSENTRY32); 
        if(Process32First(hProcessSnap, &pe32)) 
        { 
            do 
            { 
                if(lstrcmpi(pe32.szExeFile , _T("scheduler_proxy.exe")) == 0)
                {
                    bFound = TRUE;
                    break;
                }
            }
            while (Process32Next(hProcessSnap, &pe32)); 
        }
    }
    else
    {
        return 1;
    }

    if(hProcessSnap && hProcessSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hProcessSnap);
    }

    if (bFound)
    {
        return 0;
    }


	BOOL bActived = FALSE;
	const int nMaxTries = 6;
	for(int nTry=0; nTry<nMaxTries&&FALSE==bActived; ++nTry)		//try 5 times.
	{

		DWORD dwCount = -1;
		PWTS_SESSION_INFO ppSessionInfo;
		if( WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0,1, &ppSessionInfo, &dwCount) )
		{

		}else
		{
			dwCount = GetLastError();
		}

			PWTS_SESSION_INFO pSession = ppSessionInfo;
			for(int i=0;i<dwCount; ++i)
			{
				if( WTS_CONNECTSTATE_CLASS::WTSActive == pSession->State )
				{
					bActived = TRUE;
					break;
				}

				pSession++;
			}

			if( ppSessionInfo )
				WTSFreeMemory(ppSessionInfo);

			// Get token of current logon user
			HANDLE hUserToken = NULL;
			DWORD dwSessionId = WTSGetActiveConsoleSessionId();
			WTSQueryUserToken(dwSessionId, &hUserToken);

			// Launch scheduler_proxy.exe
			TCHAR szTargetExe[MAX_PATH] = {0};
		//	if( 0xFFFFFFFF == dwSessionId ) //if no user logged on, call TVTSETSCHED_EXE to write the rrcmd command to runonce registry
		//	StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\test.cmd %ld %s %ld %ld"), _tgetenv(_T("CommonProgramFiles")), pSession->SessionId, pSession->pWinStationName, pSession->State, dwCount);
		//	StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\test.cmd %s"), _tgetenv(_T("CommonProgramFiles")), bActived?_T("Actived"):_T("InActived"));
			if( !bActived )
			{
				if( nTry == nMaxTries-1 )
				{
					StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\%s setrun task=LaunchRnR"), _tgetenv(_T("CommonProgramFiles")), TVTSETSCHED_EXE);			
				}else
				{
					Sleep(4000);
					continue;
				}
				//StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\test.cmd %s"), _tgetenv(_T("CommonProgramFiles")), bActived?_T("Actived"):_T("InActived"));
			}
			else
			   StringCbPrintf(szTargetExe, sizeof(szTargetExe), _T("%s\\Lenovo\\Scheduler\\%s"), _tgetenv(_T("CommonProgramFiles")), SCHEDULER_PROXY_EXE);

			STARTUPINFO su_info;
			PROCESS_INFORMATION pr_info;
			memset(&su_info, 0, sizeof(su_info));
			su_info.dwFlags = STARTF_FORCEONFEEDBACK;
			su_info.cb = sizeof(su_info);
			su_info.lpDesktop = _T("winsta0\\default");
			memset(&pr_info, 0, sizeof(pr_info));

			// Added by daniel 2013-5-23 for fix vulnarability issue.
			TCHAR strPathwithQuotes[MAX_PATH] = {0};
			int ret = QuoteString(szTargetExe,strPathwithQuotes,MAX_PATH);
			BOOL bRet = CreateProcessAsUser(hUserToken, NULL, strPathwithQuotes, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &su_info, &pr_info);

			if(bRet == 0)
			{
				bRet = CreateProcessAsUser(hUserToken, NULL, szTargetExe, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &su_info, &pr_info);
			}
			if (!bRet)
			{
				iResult = 1;      
			}

			CloseHandle(pr_info.hProcess);
			CloseHandle(pr_info.hThread);

	}

    return iResult;

}

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               