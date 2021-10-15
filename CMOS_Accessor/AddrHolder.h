#pragma once

typedef unsigned char BYTE;

#pragma pack(push, 1)

typedef struct _address_holder
{
	BYTE address;
	BYTE data;
} ADDRRES_HOLDER, *PADDRESS_HOLDER;

#pragma pack(pop)