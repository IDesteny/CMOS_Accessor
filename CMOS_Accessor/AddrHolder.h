#pragma once
#pragma pack(push, 1)

typedef unsigned char BYTE;

typedef struct _address_holder
{
	BYTE addr;
	BYTE data;
} ADDRRES_HOLDER, *PADDRESS_HOLDER;

#pragma pack(pop)