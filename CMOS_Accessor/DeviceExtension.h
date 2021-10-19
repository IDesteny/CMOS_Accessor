#include <ntddk.h>

typedef struct _DEVICE_EXTENSION
{
	UNICODE_STRING symbolicLink;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;