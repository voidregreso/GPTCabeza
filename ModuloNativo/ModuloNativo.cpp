// ModuloNativo.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "ModuloNativo.h"
#include <string>
#pragma warning(disable:4996)

char *flipAndCodeBytes(const char *str, int pos, int flip, char *buf) {
    int i, j = 0, k = 0;
    buf[0] = '\0';
    if (pos <= 0)
        return buf;
    if (!j) {
        char p = 0;
        // First try to gather all characters representing hex digits only.
        j = 1;
        k = 0;
        buf[k] = 0;
        for (i = pos; j && str[i] != '\0'; ++i) {
            char c = tolower(str[i]);
            if (isspace(c))
                c = '0';
            ++p;
            buf[k] <<= 4;
            if (c >= '0' && c <= '9')
                buf[k] |= (unsigned char)(c - '0');
            else if (c >= 'a' && c <= 'f')
                buf[k] |= (unsigned char)(c - 'a' + 10);
            else {
                j = 0;
                break;
            }
            if (p == 2) {
                if (buf[k] != '\0' && !isprint(buf[k])) {
                    j = 0;
                    break;
                }
                ++k;
                p = 0;
                buf[k] = 0;
            }
        }
    }
    if (!j) {
        // There are non-digit characters, gather them as is.
        j = 1;
        k = 0;
        for (i = pos; j && str[i] != '\0'; ++i) {
            char c = str[i];
            if (!isprint(c)) {
                j = 0;
                break;
            }
            buf[k++] = c;
        }
    }
    if (!j) {
        // The characters are not there or are not printable.
        k = 0;
    }
    buf[k] = '\0';
    if (flip)
        // Flip adjacent characters
        for (j = 0; j < k; j += 2) {
            char t = buf[j];
            buf[j] = buf[j + 1];
            buf[j + 1] = t;
        }
    // Trim any beginning and end space
    i = j = -1;
    for (k = 0; buf[k] != '\0'; ++k) {
        if (!isspace(buf[k])) {
            if (i < 0)
                i = k;
            j = k;
        }
    }
    if ((i >= 0) && (j >= 0)) {
        for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
            buf[k - i] = buf[k];
        buf[k - i] = '\0';
    }
    return buf;
}

HANDLE AbrirDrive(U32 drvn) {
    char cur_drv[100];
    HANDLE hDrv;
    sprintf_s(cur_drv, "\\\\.\\PhysicalDrive%d", drvn);
    hDrv = CreateFile(cur_drv, GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING, 0, NULL
                     );
    return hDrv;
}

BOOL JudgeExist(U32 drvn) {
    HANDLE hdrv = AbrirDrive(drvn);
    if (hdrv == INVALID_HANDLE_VALUE || GetLastError() == 2) {
        CloseHandle(hdrv);
        return FALSE;
    }
    CloseHandle(hdrv);
    return TRUE;
}

BOOL JudgeIfGPT(U32 drvn) {
    DWORD leido;
    DWORD szNewLayout = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + 128 * sizeof(PARTITION_INFORMATION_EX);
    DRIVE_LAYOUT_INFORMATION_EX *dlie = (DRIVE_LAYOUT_INFORMATION_EX *) new BYTE[szNewLayout];
    ZeroMemory(dlie, szNewLayout);
    HANDLE hdrv = AbrirDrive(drvn);
    if (hdrv == INVALID_HANDLE_VALUE) {
        CloseHandle(hdrv);
        return FALSE;
    }
    BOOL res = DeviceIoControl(hdrv, IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                               NULL, 0, dlie, szNewLayout, &leido, NULL
                              );
    if (!res) {
        CloseHandle(hdrv);
        return FALSE;
    }
    CloseHandle(hdrv);
    return (dlie->PartitionStyle == PARTITION_STYLE_GPT);
}

void GetDiscoName(U32 drvn, char *pNom) {
    HANDLE hdrv = AbrirDrive(drvn);
    char modelNumber[1000];
    char vendorId[1000];
    if (hdrv == INVALID_HANDLE_VALUE) {
        CloseHandle(hdrv);
        return;
    }
    STORAGE_PROPERTY_QUERY query;
    DWORD cbBytesReturned = 0;
    char local_buffer[10000];
    memset((void *)&query, 0, sizeof(query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;
    memset(local_buffer, 0, sizeof(local_buffer));
    BOOL res = DeviceIoControl(hdrv, IOCTL_STORAGE_QUERY_PROPERTY,
                               &query,
                               sizeof(query),
                               &local_buffer[0],
                               sizeof(local_buffer),
                               &cbBytesReturned, NULL);
    if (!res) {
        CloseHandle(hdrv);
        return;
    }
    STORAGE_DEVICE_DESCRIPTOR *descrip = (STORAGE_DEVICE_DESCRIPTOR *)&local_buffer;
    flipAndCodeBytes(local_buffer, descrip->VendorIdOffset, 0, vendorId);
    flipAndCodeBytes(local_buffer, descrip->ProductIdOffset, 0, modelNumber);
    strcpy(pNom, vendorId);
    strcat(pNom, " ");
    strcat(pNom, modelNumber);
}

U32 HDD_read(U32 drv, U32 SecAddr, U32 blocks, U8 *buf) {
    U32 ret = 0;
    U32 ldistanceLow, ldistanceHigh, dwpointer, bytestoread, numread;
    char cur_drv[100];
    HANDLE g_hDevice;
    sprintf(cur_drv, "\\\\.\\PhysicalDrive%d", drv);
    g_hDevice = CreateFile(cur_drv, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (g_hDevice == INVALID_HANDLE_VALUE)	return 0;
    ldistanceLow = SecAddr << 9;
    ldistanceHigh = SecAddr >> (32 - 9);
    dwpointer = SetFilePointer(g_hDevice, ldistanceLow, (long *)&ldistanceHigh, FILE_BEGIN);
    if (dwpointer != 0xFFFFFFFF) {
        bytestoread = blocks * 512;
        ret = ReadFile(g_hDevice, buf, bytestoread, (unsigned long *)&numread, NULL);
        if (ret)	ret = 1;
        else		ret = 0;
    }
    CloseHandle(g_hDevice);
    return ret;
}

BOOL GetGPTHeader(U32 nDrv, U32 nSecPos, PGPT_HEADER pGPTHeader) {
    U8 pSecBuf[512];
    if (HDD_read(nDrv, nSecPos, 1, pSecBuf) == 0) {
        printf("Primary GPT Header read failed \n");
        return false;
    }
    memcpy(pGPTHeader, pSecBuf, sizeof(GPT_HEADER));
    return true;
}