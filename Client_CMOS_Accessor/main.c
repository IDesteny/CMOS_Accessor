#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "..\CMOS_Accessor\OIctl.h"
#include "..\CMOS_Accessor\AddrHolder.h"
#include "..\CMOS_Accessor\DriverCfg.h"

#define DEVICE_PATH _T("\\\\.\\") DEVICE_NAME

INT WINAPI _tmain(INT argc, LPCTSTR argv[])
{
	HANDLE hDriver = CreateFile(
		DEVICE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hDriver == INVALID_HANDLE_VALUE)
	{
		_ftprintf_s(stderr, _T("ERROR! 'CreateFile()' code=%lu\n"), GetLastError());
		return EXIT_FAILURE;
	}

	_tprintf(_T("=== CMOS memory accessor ===\n"));

	while (TRUE)
	{
		_tprintf(_T("> "));

		TCHAR cmd[4] = { 0 };
		_tscanf_s(_T("%s"), cmd, (UINT)ARRAYSIZE(cmd));

		if (!_tcscmp(cmd, _T("hlp")))
		{
			_tprintf(_T("get <addr>\n"));
			_tprintf(_T("set <addr> <val>\n"));
			_tprintf(_T("cls\n"));
			_tprintf(_T("ext\n"));
			continue;
		}

		if (!_tcscmp(cmd, _T("cls")))
		{
			_tsystem(_T("cls"));
			_tprintf(_T("=== CMOS memory accessor ===\n"));
			continue;
		}

		if (!_tcscmp(cmd, _T("ext")))
		{
			break;
		}

		if (!_tcscmp(cmd, _T("get")))
		{
			TCHAR addr[16] = { 0 };
			_tscanf_s(_T("%s"), addr, (UINT)ARRAYSIZE(addr));

			PTCHAR correctAddr;
			ADDRRES_HOLDER addressHolder;

			addressHolder.addr = (UINT8)_tcstol(addr, &correctAddr, 16);
			if (*correctAddr || addressHolder.addr > 0x3f)
			{
				_tprintf(_T("Invalid address\n"));
				continue;
			}

			DWORD ret;
			if (!DeviceIoControl(
				hDriver, GET_DATA_BY_ADDR,
				&addressHolder.addr, sizeof addressHolder.addr,
				&addressHolder.data, sizeof addressHolder.data,
				&ret, NULL))
			{
				_ftprintf_s(stderr, _T("ERROR! 'DeviceIoControl()' code=%lu\n"), GetLastError());
				return EXIT_FAILURE;
			}

			if (ret != sizeof(UINT8))
			{
				_ftprintf_s(stderr, _T("ERROR! Incorrect return data in 'DeviceIoControl()'\n"));
				continue;
			}

			_tprintf(_T("data: %hhx\n"), addressHolder.data);

			continue;
		}

		if (!_tcscmp(cmd, _T("set")))
		{
			TCHAR addr[16] = { 0 };
			_tscanf_s(_T("%s"), addr, (UINT)ARRAYSIZE(addr));
			
			TCHAR data[16] = { 0 };
			_tscanf_s(_T("%s"), data, (UINT)ARRAYSIZE(data));

			PTCHAR correctNumb;
			ADDRRES_HOLDER addressHolder;

			addressHolder.addr = (UINT8)_tcstol(addr, &correctNumb, 16);
			if (*correctNumb || addressHolder.addr > 0x3f)
			{
				_tprintf(_T("Invalid address\n"));
				continue;
			}

			addressHolder.data = (UINT8)_tcstol(data, &correctNumb, 16);
			if (*correctNumb)
			{
				_tprintf(_T("Invalid data\n"));
				continue;
			}

			DWORD ret;
			if (!DeviceIoControl(
				hDriver, SET_DATA_BY_ADDR,
				&addressHolder, sizeof addressHolder,
				NULL, 0,
				&ret, NULL))
			{
				_ftprintf_s(stderr, _T("ERROR! 'DeviceIoControl()' code=%lu\n"), GetLastError());
				return EXIT_FAILURE;
			}

			if (ret)
			{
				_ftprintf_s(stderr, _T("ERROR! Incorrect return data in 'DeviceIoControl()'\n"));
			}

			continue;
		}

		_tprintf(_T("Unknown command\n"));
		_tprintf(_T("For help: 'hlp'\n"));
	}

	return EXIT_SUCCESS;
}