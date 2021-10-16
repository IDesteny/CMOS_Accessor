#include <ntddk.h>
#include <tchar.h>
#include "OIctl.h"
#include "AddrHolder.h"
#include "DriverCfg.h"

#define ERRLOG(msg) DbgPrint("ERROR! ===" msg "===")

#define DEVICE_PATH _T("\\Device\\") DEVICE_NAME
#define	SYM_LINK_NAME _T("\\DosDevices\\") DEVICE_NAME


NTSTATUS CompleteIrp(IN PIRP pIrp, IN NTSTATUS ntStatus, IN ULONG info)
{
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = info;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

NTSTATUS DeviceControlRoutine(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG controlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	NTSTATUS ntStatusExitCode = STATUS_SUCCESS;
	ULONG bytesTxd = 0;

	switch (controlCode)
	{
		case GET_DATA_BY_ADDR:
		{
			PUINT8 addr = pIrp->AssociatedIrp.SystemBuffer;

			if (!addr)
			{
				ntStatusExitCode = STATUS_BUFFER_ALL_ZEROS;
				ERRLOG("DeviceControlRoutine()");
				break;
			}

			WRITE_PORT_UCHAR((PUINT8)0x70, *addr);
			*addr = READ_PORT_UCHAR((PUINT8)0x71);

			bytesTxd = sizeof(UINT8);
		}
		break;

		case SET_DATA_BY_ADDR:
		{
			PADDRESS_HOLDER pAddressHolder = pIrp->AssociatedIrp.SystemBuffer;

			if (!pAddressHolder)
			{
				ntStatusExitCode = STATUS_BUFFER_ALL_ZEROS;
				ERRLOG("DeviceControlRoutine()");
				break;
			}

			WRITE_PORT_UCHAR((PUINT8)0x70, pAddressHolder->addr);
			WRITE_PORT_UCHAR((PUINT8)0x71, pAddressHolder->data);
		}
		break;

		default:
		{
			ntStatusExitCode = STATUS_INVALID_PARAMETER;
			ERRLOG("DeviceControlRoutine()");
		}
	}

	return CompleteIrp(pIrp, ntStatusExitCode, bytesTxd);
}

NTSTATUS CreateFileRoutine(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);
}

NTSTATUS CloseFileRoutine(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);
}

VOID UnloadRoutine(IN PDRIVER_OBJECT pDriverObject)
{
	if (!NT_SUCCESS(IoDeleteSymbolicLink(pDriverObject->DeviceObject->DeviceExtension)))
	{
		ERRLOG("IoDeleteSymbolicLink()");
	}

	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING RegistryPath)
{
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlRoutine;
	pDriverObject->DriverUnload = UnloadRoutine;

	UNICODE_STRING deviceName;
	RtlInitUnicodeString(&deviceName, DEVICE_PATH);

	PDEVICE_OBJECT deviceObj;
	NTSTATUS lastStatus = STATUS_SUCCESS;

	lastStatus = IoCreateDevice(pDriverObject,
		sizeof(UNICODE_STRING),
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObj);

	if (!NT_SUCCESS(lastStatus))
	{
		ERRLOG("IoCreateDevice()");
		return lastStatus;
	}
	
	RtlInitUnicodeString(deviceObj->DeviceExtension, SYM_LINK_NAME);

	lastStatus = IoCreateSymbolicLink(deviceObj->DeviceExtension, &deviceName);
	if (!NT_SUCCESS(lastStatus))
	{
		ERRLOG("IoCreateSymbolicLink()");
		return lastStatus;
	}

	return lastStatus;
}