#pragma once
#pragma pack(push, 1)

typedef unsigned char BYTE;

typedef struct _ADDRESS_HOLDER
{
	BYTE addr;
	BYTE data;
} ADDRRES_HOLDER, *PADDRESS_HOLDER;

#pragma pack(pop)