#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <string>


#define Win32Throw(code, msg) {printf("error(%d): %s\n", code, msg); return 0;}



BOOL InjectDllToProcess(HANDLE hProcess, LPCTSTR lpDllPath){
	SIZE_T iSize = (lstrlen(lpDllPath) + 1 )* sizeof(TCHAR);
	SIZE_T iSizeWrited = 0;

	LPVOID lpRemoteBuf = VirtualAllocEx(hProcess, NULL, iSize, MEM_COMMIT, PAGE_READWRITE);
	if (!lpRemoteBuf)Win32Throw(GetLastError(), "VirtualAllocEx");

	BOOL bret = WriteProcessMemory(hProcess, lpRemoteBuf, lpDllPath, iSize, &iSizeWrited);
	if (!bret) {
		int e = GetLastError();
		VirtualFreeEx(hProcess, lpRemoteBuf, iSize, MEM_COMMIT);
		Win32Throw(e, "WriteProcessMemory");
	}

	DWORD dwNewThreadId = 0;
	LPVOID lpLoadDll = LoadLibrary;
	HANDLE hNewRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpLoadDll, lpRemoteBuf, 0, &dwNewThreadId);
	if (!hNewRemoteThread)	{
		int e = GetLastError();
		VirtualFreeEx(hProcess, lpRemoteBuf, iSize, MEM_COMMIT);
		Win32Throw(e, "CreateRemoteThread");
	}

	WaitForSingleObject(hNewRemoteThread, INFINITE);
	return TRUE;
}

BOOL InjectDllWithProcessID(DWORD pid, LPCTSTR lpDllPath) {
	BOOL ret = FALSE;
	HANDLE hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hp)Win32Throw(GetLastError(), "OpenProcess");
	ret = InjectDllToProcess(hp, lpDllPath);
	CloseHandle(hp);
	return ret;
}


BOOL InjectDllWithProcessName(LPCTSTR lpName, LPCTSTR lpDllPath) {
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap)Win32Throw(GetLastError(), "CreateToolhelp32Snapshot");
	BOOL ret = FALSE;
	PROCESSENTRY32 psentry;
	memset(&psentry, 0, sizeof(psentry));
	psentry.dwSize = sizeof(psentry);

	if (!Process32First(hProcessSnap, &psentry)) {
		int e = GetLastError();
		CloseHandle(hProcessSnap);
		Win32Throw(e, "Process32First");
	}

	int namelen = lstrlen(lpName);
	do {
		TCHAR* lpExeName = psentry.szExeFile;
		int pathlen = lstrlen(lpExeName);
		if (pathlen > namelen) {
			lpExeName += (pathlen - namelen);
		}
		if (_tcsicmp(lpExeName, lpName) == 0) {
			printf("%d\n", psentry.th32ProcessID);
			ret = ret && InjectDllWithProcessID(psentry.th32ProcessID, lpDllPath);
		}
	} while (Process32Next(hProcessSnap, &psentry));

	CloseHandle(hProcessSnap);
	return ret;
}



int _tmain(int argc, TCHAR* argv[]) {
	if (argc > 2) {
		BOOL bRet = FALSE;
		LPCTSTR lpPid = argv[1];
		LPCTSTR lpDll = argv[2];
		int iPid = _ttoi(lpPid);
		if (iPid) {
			bRet = InjectDllWithProcessID(iPid, lpDll);
		}
		else {
			bRet = InjectDllWithProcessName(lpPid, lpDll);
		}
		if (!bRet) {
			printf("inject error %d\n",GetLastError());
			return 1;
		}
		return 0;
	}
	else {
		printf("usage: injectdll <process> <dll>\n");
		printf("  process can be exe name or pid.\n");
		printf("  dll must be full path.\n");
	}

	return 1;
}
