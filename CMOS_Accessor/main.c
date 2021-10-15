#include <ntddk.h>
#include <tchar.h>
#include "OIctl.h"
#include "AddrHolder.h"
#include "DriverCfg.h"

#define ERROR_LOG(msg) DbgPrint("ERROR! === " msg " ===")
#define SUCCESS_LOG(msg) DbgPrint("SUCCESS! === " msg " ===")

#define DEVICE_PATH _T("\\Device\\") DEVICE_NAME
#define	SYM_LINK_NAME _T("\\DosDevices\\") DEVICE_NAME

UNICODE_STRING symbolicLink;

NTSTATUS CompleteIrp(IN CONST PIRP pIrp, CONST NTSTATUS ntStatus, IN CONST ULONG info)
{
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = info;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

NTSTATUS DeviceControlRoutine(IN CONST PDEVICE_OBJECT pDeviceObject, IN CONST PIRP pIrp)
{
	CONST PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	CONST ULONG controlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	NTSTATUS ntStatusExitCode = STATUS_SUCCESS;
	ULONG bytesTxd = 0;

	switch (controlCode)
	{
	case GETTER:
	{
		CONST PUINT8 addr = pIrp->AssociatedIrp.SystemBuffer;

		WRITE_PORT_UCHAR((PUINT8)0x70, *addr);
		*addr = READ_PORT_UCHAR((PUINT8)0x71);

		bytesTxd = sizeof(UINT8);
	}
	break;

	case SETTER:
	{
		CONST PUINT8 addr = pIrp->AssociatedIrp.SystemBuffer;

		WRITE_PORT_UCHAR((PUINT8)0x70, *addr);
		WRITE_PORT_UCHAR((PUINT8)0x71, addr[1]);
	}
	break;

	default:
	{
		ntStatusExitCode = STATUS_INVALID_PARAMETER;
	}
	break;
	}

	return CompleteIrp(pIrp, ntStatusExitCode, bytesTxd);
}

NTSTATUS CreateFileRoutine(IN CONST PDEVICE_OBJECT pDeviceObject, IN CONST PIRP pIrp)
{
	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);
}

NTSTATUS CloseFileRoutine(IN CONST PDEVICE_OBJECT pDeviceObject, IN CONST PIRP pIrp)
{
	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);
}

VOID UnloadRoutine(IN CONST PDRIVER_OBJECT pDriverObject)
{
	IoDeleteDevice(pDriverObject->DeviceObject);
	CONST NTSTATUS ntStatusResultIoDeleteSymbolicLink = IoDeleteSymbolicLink(&symbolicLink);
	if (ntStatusResultIoDeleteSymbolicLink != STATUS_SUCCESS)
	{
		ERROR_LOG("IoDeleteSymbolicLink()");
		return;
	}
	SUCCESS_LOG("Driver stopped");
}

NTSTATUS DriverEntry(IN CONST PDRIVER_OBJECT pDriverObject, IN CONST PUNICODE_STRING RegistryPath)
{
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlRoutine;
	pDriverObject->DriverUnload = UnloadRoutine;

	UNICODE_STRING deviceName;
	RtlInitUnicodeString(&deviceName, DEVICE_PATH);

	PDEVICE_OBJECT deviceObj;
	CONST NTSTATUS ntStatusResultIoCreateDevice = IoCreateDevice(pDriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObj);
	if (ntStatusResultIoCreateDevice != STATUS_SUCCESS)
	{
		ERROR_LOG("IoCreateDevice()");
		return ntStatusResultIoCreateDevice;
	}

	RtlInitUnicodeString(&symbolicLink, SYM_LINK_NAME);

	CONST NTSTATUS ntStatusResultIoCreateSymbolicLink = IoCreateSymbolicLink(&symbolicLink, &deviceName);
	if (ntStatusResultIoCreateSymbolicLink != STATUS_SUCCESS)
	{
		ERROR_LOG("IoCreateSymbolicLink()");
		return ntStatusResultIoCreateSymbolicLink;
	}

	SUCCESS_LOG("Driver started");
	return STATUS_SUCCESS;
}