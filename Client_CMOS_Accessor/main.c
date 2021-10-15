#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define IOCTL(CODE) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800 + CODE, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define GETTER IOCTL(0x01)
#define SETTER IOCTL(0x02)

#define DEVICE_PATH _T("\\\\.\\exampledevice")
#define DOCS_PATH _T("http://philipstorr.id.au/pcbook/book5/cmoslist.htm")


INT WINAPI _tmain(CONST INT argc, LPCTSTR argv[])
{
	CONST HANDLE hDriver = CreateFile(
		DEVICE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hDriver == INVALID_HANDLE_VALUE)
	{
		_ftprintf_s(stderr, _T("ERROR! 'CreateFile()' code=%d"), GetLastError());
		return EXIT_FAILURE;
	}

	_putts(_T("=== CMOS memory accessor ==="));

	while (TRUE)
	{
		_tprintf(_T("> "));

		TCHAR cmd[16] = { 0 };
		_tscanf_s(_T("%s"), cmd, (UINT)(sizeof cmd / sizeof(TCHAR)));

		if (!_tcscmp(cmd, _T("help")))
		{
			_putts(_T("get <addr>"));
			_putts(_T("set <addr> <val>"));
			_putts(_T("doc"));
			_putts(_T("cls"));
			continue;
		}

		if (!_tcscmp(cmd, _T("doc")))
		{
			ShellExecute(0, 0, DOCS_PATH, 0, 0, SW_SHOW);
			continue;
		}

		if (!_tcscmp(cmd, _T("cls")))
		{
			_tsystem(_T("cls"));
			_putts(_T("=== CMOS memory accessor ==="));
			continue;
		}

		if (!_tcscmp(cmd, _T("get")))
		{
			UINT8 addr, data;
			_tscanf_s(_T("%hhx"), &addr);

			DWORD ret = 0;
			CONST BOOL bResultDeviceIoControl = DeviceIoControl(
				hDriver, GETTER,
				&addr, sizeof addr,
				&data, sizeof data,
				&ret, NULL);

			_tprintf(_T("data: %hhx\n"), data);
			continue;
		}

		if (!_tcscmp(cmd, _T("set")))
		{
			UINT8 addr, data;
			_tscanf_s(_T("%hhx%hhx"), &addr, &data);

			BOOLEAN verify = 0;
			_tprintf(_T("Are you sure you want to change the value? (1/0): "));
			_tscanf_s(_T("%hhu"), &verify);

			if (verify)
			{
				_putts(_T("Data changed"));
			#if FALSE
				DWORD ret = 0;
				CONST BOOL bResultDeviceIoControl = DeviceIoControl(
					hDriver, SETTER,
					&addr, sizeof addr,
					&data, sizeof data,
					&ret, NULL);
			#endif
			}
			continue;
		}

		_putts(_T("Unknown command"));
		_putts(_T("For help: 'help'"));
	}

	CloseHandle(hDriver);
	return EXIT_SUCCESS;
}