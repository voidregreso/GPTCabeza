using System;
using System.Runtime.InteropServices;
using System.Text;

namespace GPTCabeza_CRC32
{
    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
    public struct GPT_HEADER
    {
        public ulong signature; // "EFI PART" signature
        public uint revision; // Version 1.0
        public uint header_size; // size of header, generally 0x5c(92 in decimal) byte
        public uint header_crc32; // crc32 of header
        public uint reserved; // reserved, not used actually
        public ulong header_lba; // LBA of gpt header, always 0x01
        public ulong header_backup_lba; // LBA of backup gpt header
        public ulong partition_start; // starting LBA for partitions, (primary partition table last LBA)+1
        public ulong partition_end; // ending LBA for partitions
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] guid;
        public ulong entry_start; // starting LBA address of partition entry tables
        public uint entry_max_num; // maximum number of supported partition entries, generally 0x80(128 in dec)
        public uint entry_size; // size of partiion table entry, generally 128
        public uint table_crc32; // crc32 of partition table
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 420)]
        public byte[] reserved2;
    };

    public class Nativo
    {

        [DllImport("ModuloNativo.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern bool JudgeExist(uint udrv);
        [DllImport("ModuloNativo.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern bool JudgeIfGPT(uint udrv);
        [DllImport("ModuloNativo.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void GetDiscoName(uint udrv, StringBuilder sb);
        [DllImport("ModuloNativo.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern bool GetGPTHeader(uint ndrv, uint secpos, ref GPT_HEADER gpt);
    }
}
