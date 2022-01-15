#ifndef __MODULONATIVO_H_
#define __MODULONATIVO_H_

#ifdef MODULONATIVO_EXPORTS
#define MODULONATIVO_API __declspec(dllexport)
#else
#define MODULONATIVO_API __declspec(dllimport)
#endif

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long int U64;

#pragma pack(push,1)
typedef struct _GPT_HEADER {
    U64 signature; // "EFI PART" signature
    U32 revision; // Version 1.0
    U32 header_size; // size of header, generally 0x5c(92 in decimal) byte
    U32 header_crc32; // crc32 of header
    U32 reserved; // reserved, not used actually
    U64 header_lba; // LBA of gpt header, always 0x01
    U64 header_backup_lba; // LBA of backup gpt header
    U64 partition_start; // starting LBA for partitions, (primary partition table last LBA)+1
    U64 partition_end; // ending LBA for partitions
    U8 guid[16]; // disk guid
    U64 entry_start; // starting LBA address of partition entry tables
    U32 entry_max_num; // maximum number of supported partition entries, generally 0x80(128 in dec)
    U32 entry_size; // size of partiion table entry, generally 128
    U32 table_crc32; // crc32 of partition table
    U8 reserved2[420]; // reserved bytes, not used actually
} GPT_HEADER, * PGPT_HEADER;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _MYGUID {
    U32 segment1;
    U16 segment2;
    U16 segment3;
    U8 segment4[8];
} MYGUID, * PMYGUID;
#pragma pack(pop)

HANDLE AbrirDrive(U32 drvn);
char* flipAndCodeBytes(const char* str, int pos, int flip, char* buf);
U32 HDD_read(U32 drv, U32 SecAddr, U32 blocks, U8* buf);
extern "C" MODULONATIVO_API BOOL __cdecl JudgeExist(U32 drvn);
extern "C" MODULONATIVO_API BOOL __cdecl JudgeIfGPT(U32 drvn);
extern "C" MODULONATIVO_API void __cdecl GetDiscoName(U32 drvn, char* pNom);
extern "C" MODULONATIVO_API BOOL __cdecl GetGPTHeader(U32 nDrv, U32 nSecPos, PGPT_HEADER pGPTHeader);
#endif