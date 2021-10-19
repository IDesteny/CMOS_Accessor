#include <ntddk.h>
#include <tchar.h>

#include "IOctl.h"
#include "AddrHolder.h"
#include "DriverCfg.h"
#include "DeviceExtension.h"

#define ERRLOG(msg) DbgPrint("ERROR! ===" msg "===")

#define DEVICE_PATH _T("\\Device\\") DEVICE_NAME
#define SYM_LINK_NAME _T("\\DosDevices\\") DEVICE_NAME


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
				ERRLOG("DeviceControlRoutine()");
				ntStatusExitCode = STATUS_BUFFER_ALL_ZEROS;
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
				ERRLOG("DeviceControlRoutine()");
				ntStatusExitCode = STATUS_BUFFER_ALL_ZEROS;
				break;
			}

			WRITE_PORT_UCHAR((PUINT8)0x70, pAddressHolder->addr);
			WRITE_PORT_UCHAR((PUINT8)0x71, pAddressHolder->data);
		}
		break;

		default:
		{
			ERRLOG("DeviceControlRoutine()");
			ntStatusExitCode = STATUS_INVALID_PARAMETER;
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
	PDEVICE_EXTENSION pDeviceExtension = pDriverObject->DeviceObject->DeviceExtension;

	IoDeleteSymbolicLink(&pDeviceExtension->symbolicLink);
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING RegistryPath)
{
	if (!pDriverObject)
		return STATUS_BUFFER_ALL_ZEROS;

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseFileRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlRoutine;
	pDriverObject->DriverUnload = UnloadRoutine;

	UNICODE_STRING deviceName;
	RtlInitUnicodeString(&deviceName, DEVICE_PATH);

	NTSTATUS returnStatus;
	PDEVICE_OBJECT deviceObj;

	returnStatus = IoCreateDevice(
		pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObj);

	if (!NT_SUCCESS(returnStatus))
		return returnStatus;

	if (!deviceObj)
		return STATUS_BUFFER_ALL_ZEROS;
	
	PDEVICE_EXTENSION pDeviceExtension = deviceObj->DeviceExtension;
	if (!pDeviceExtension)
	{
		IoDeleteDevice(deviceObj);
		return STATUS_BUFFER_ALL_ZEROS;
	}

	RtlInitUnicodeString(&pDeviceExtension->symbolicLink, SYM_LINK_NAME);

	returnStatus = IoCreateSymbolicLink(&pDeviceExtension->symbolicLink, &deviceName);
	if (!NT_SUCCESS(returnStatus))
	{
		IoDeleteDevice(deviceObj);
		return returnStatus;
	}

	return STATUS_SUCCESS;
}