#include "implode.h"
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Local structures

// Compression structure (Size 12596 bytes)
typedef struct {
    unsigned long offs0000;   // 0000
    unsigned long ctype;      // 0004 - Compression type (CMP_BINARY or CMP_ASCII)
    unsigned long outputPos;  // 0008 - Position in output buffer
    unsigned long dsize_bits; // 000C - Dict size (4, 5, 6 for 0x400, 0x800, 0x1000)
    unsigned long dsize_mask; // 0010 - Dict size bitmask (0x0F, 0x1F, 0x3F for 0x400, 0x800, 0x1000)
    unsigned long bit_buff;   // 0014 - 16-bit buffer for processing input data
    unsigned long extra_bits; // 0018 - Number of extra (above 8) bits in bit buffer
    unsigned int  in_pos;     // 001C - Position in in_buff
    unsigned long in_bytes;   // 0020 - Number of bytes in input buffer
    void         *param;      // 0024 - Custom parameter
    unsigned int(PKEXPORT *read_buf)(char *buf, unsigned int *size, void *param); // 0028
    void(PKEXPORT *write_buf)(char *buf, unsigned int *size, void *param);        // 002C
    unsigned char  out_buff[0x2000]; // 0030 - Output circle buffer. Starting position is 0x1000
    unsigned char  offs2030[0x204];  // 2030 - ???
    unsigned char  in_buff[0x800];   // 2234 - Buffer for data to be decompressed
    unsigned char  position1[0x100]; // 2A34 - Positions in buffers
    unsigned char  position2[0x100]; // 2B34 - Positions in buffers
    unsigned char  offs2C34[0x100];  // 2C34 - Buffer for
    unsigned char  offs2D34[0x100];  // 2D34 - Buffer for
    unsigned char  offs2E34[0x80];   // 2EB4 - Buffer for
    unsigned char  offs2EB4[0x100];  // 2EB4 - Buffer for
    unsigned char  ChBitsAsc[0x100]; // 2FB4 - Buffer for
    unsigned char  DistBits[0x40];   // 30B4 - Numbers of bytes to skip copied block length
    unsigned char  LenBits[0x10];    // 30F4 - Numbers of bits for skip copied block length
    unsigned char  ExLenBits[0x10];  // 3104 - Number of valid bits for copied block
    unsigned short LenBase[0x10];    // 3114 - Buffer for
} TDcmpStruct;

//-----------------------------------------------------------------------------
// Tables

static unsigned char DistBits[] = {0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                                   0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07,
                                   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                                   0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08,
                                   0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};

static unsigned char DistCode[] = {0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E, 0x1E, 0x2E, 0x0E, 0x36, 0x16,
                                   0x26, 0x06, 0x3A, 0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02, 0x7C, 0x3C,
                                   0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C, 0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44,
                                   0x04, 0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08, 0xF0, 0x70, 0xB0, 0x30,
                                   0xD0, 0x50, 0x90, 0x10, 0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00};

static unsigned char ExLenBits[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

static unsigned short LenBase[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                   0x0008, 0x000A, 0x000E, 0x0016, 0x0026, 0x0046, 0x0086, 0x0106};

static unsigned char LenBits[] = {0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05,
                                  0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07};

static unsigned char LenCode[] = {0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14,
                                  0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00};

static unsigned char ChBitsAsc[] = {
    0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x08, 0x07, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x04, 0x0A, 0x08, 0x0C, 0x0A, 0x0C,
    0x0A, 0x08, 0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08, 0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07,
    0x08, 0x08, 0x0C, 0x0B, 0x07, 0x09, 0x0B, 0x0C, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08, 0x08, 0x06, 0x0B, 0x09,
    0x06, 0x07, 0x06, 0x06, 0x07, 0x0B, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x09, 0x09, 0x0B, 0x08, 0x0B, 0x09, 0x0C,
    0x08, 0x0C, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x0B, 0x07, 0x05, 0x06, 0x05, 0x05, 0x06, 0x0A,
    0x05, 0x05, 0x05, 0x05, 0x08, 0x07, 0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0C, 0x0D, 0x0D,
    0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D};

static unsigned short ChCodeAsc[] = {
    0x0490, 0x0FE0, 0x07E0, 0x0BE0, 0x03E0, 0x0DE0, 0x05E0, 0x09E0, 0x01E0, 0x00B8, 0x0062, 0x0EE0, 0x06E0, 0x0022,
    0x0AE0, 0x02E0, 0x0CE0, 0x04E0, 0x08E0, 0x00E0, 0x0F60, 0x0760, 0x0B60, 0x0360, 0x0D60, 0x0560, 0x1240, 0x0960,
    0x0160, 0x0E60, 0x0660, 0x0A60, 0x000F, 0x0250, 0x0038, 0x0260, 0x0050, 0x0C60, 0x0390, 0x00D8, 0x0042, 0x0002,
    0x0058, 0x01B0, 0x007C, 0x0029, 0x003C, 0x0098, 0x005C, 0x0009, 0x001C, 0x006C, 0x002C, 0x004C, 0x0018, 0x000C,
    0x0074, 0x00E8, 0x0068, 0x0460, 0x0090, 0x0034, 0x00B0, 0x0710, 0x0860, 0x0031, 0x0054, 0x0011, 0x0021, 0x0017,
    0x0014, 0x00A8, 0x0028, 0x0001, 0x0310, 0x0130, 0x003E, 0x0064, 0x001E, 0x002E, 0x0024, 0x0510, 0x000E, 0x0036,
    0x0016, 0x0044, 0x0030, 0x00C8, 0x01D0, 0x00D0, 0x0110, 0x0048, 0x0610, 0x0150, 0x0060, 0x0088, 0x0FA0, 0x0007,
    0x0026, 0x0006, 0x003A, 0x001B, 0x001A, 0x002A, 0x000A, 0x000B, 0x0210, 0x0004, 0x0013, 0x0032, 0x0003, 0x001D,
    0x0012, 0x0190, 0x000D, 0x0015, 0x0005, 0x0019, 0x0008, 0x0078, 0x00F0, 0x0070, 0x0290, 0x0410, 0x0010, 0x07A0,
    0x0BA0, 0x03A0, 0x0240, 0x1C40, 0x0C40, 0x1440, 0x0440, 0x1840, 0x0840, 0x1040, 0x0040, 0x1F80, 0x0F80, 0x1780,
    0x0780, 0x1B80, 0x0B80, 0x1380, 0x0380, 0x1D80, 0x0D80, 0x1580, 0x0580, 0x1980, 0x0980, 0x1180, 0x0180, 0x1E80,
    0x0E80, 0x1680, 0x0680, 0x1A80, 0x0A80, 0x1280, 0x0280, 0x1C80, 0x0C80, 0x1480, 0x0480, 0x1880, 0x0880, 0x1080,
    0x0080, 0x1F00, 0x0F00, 0x1700, 0x0700, 0x1B00, 0x0B00, 0x1300, 0x0DA0, 0x05A0, 0x09A0, 0x01A0, 0x0EA0, 0x06A0,
    0x0AA0, 0x02A0, 0x0CA0, 0x04A0, 0x08A0, 0x00A0, 0x0F20, 0x0720, 0x0B20, 0x0320, 0x0D20, 0x0520, 0x0920, 0x0120,
    0x0E20, 0x0620, 0x0A20, 0x0220, 0x0C20, 0x0420, 0x0820, 0x0020, 0x0FC0, 0x07C0, 0x0BC0, 0x03C0, 0x0DC0, 0x05C0,
    0x09C0, 0x01C0, 0x0EC0, 0x06C0, 0x0AC0, 0x02C0, 0x0CC0, 0x04C0, 0x08C0, 0x00C0, 0x0F40, 0x0740, 0x0B40, 0x0340,
    0x0300, 0x0D40, 0x1D00, 0x0D00, 0x1500, 0x0540, 0x0500, 0x1900, 0x0900, 0x0940, 0x1100, 0x0100, 0x1E00, 0x0E00,
    0x0140, 0x1600, 0x0600, 0x1A00, 0x0E40, 0x0640, 0x0A40, 0x0A00, 0x1200, 0x0200, 0x1C00, 0x0C00, 0x1400, 0x0400,
    0x1800, 0x0800, 0x1000, 0x0000};

//-----------------------------------------------------------------------------
// Local variables

static char copyright[] = "PKWARE Data Compression Library for Win32\r\n"
                          "Copyright 1989-1995 PKWARE Inc.  All Rights Reserved\r\n"
                          "Patent No. 5,051,745\r\n"
                          "PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.\r\n"
                          "Version 1.11";

//-----------------------------------------------------------------------------
// Local functions

static void gen_decode_tabs(const long count, const unsigned char *bits, const unsigned char *p_code,
                            unsigned char *buffer2) {

    for (long i = count - 1; i >= 0; i--) // EBX - count
    {
        unsigned long       idx1 = p_code[i];
        const unsigned long idx2 = 1 << bits[i];

        do {
            buffer2[idx1]  = (unsigned char)i;
            idx1          += idx2;
        } while (idx1 < 0x100);
    }
}

static void gen_asc_tabs(TDcmpStruct *p_work) {
    unsigned short *p_ch_code_asc = &ChCodeAsc[0xFF];
    unsigned long   acc, add;

    for (unsigned short count = 0x00FF; p_ch_code_asc >= ChCodeAsc; p_ch_code_asc--, count--) {
        unsigned char *p_ch_bits_asc = p_work->ChBitsAsc + count;
        unsigned char  bits_asc      = *p_ch_bits_asc;

        if (bits_asc <= 8) {
            add = (1 << bits_asc);
            acc = *p_ch_code_asc;

            do {
                p_work->offs2C34[acc]  = (unsigned char)count;
                acc                   += add;
            } while (acc < 0x100);
        } else if ((acc = (*p_ch_code_asc & 0xFF)) != 0) {
            p_work->offs2C34[acc] = 0xFF;

            if (*p_ch_code_asc & 0x3F) {
                bits_asc       -= 4;
                *p_ch_bits_asc  = bits_asc;

                add = (1 << bits_asc);
                acc = *p_ch_code_asc >> 4;
                do {
                    p_work->offs2D34[acc]  = (unsigned char)count;
                    acc                   += add;
                } while (acc < 0x100);
            } else {
                bits_asc       -= 6;
                *p_ch_bits_asc  = bits_asc;

                add = (1 << bits_asc);
                acc = *p_ch_code_asc >> 6;
                do {
                    p_work->offs2E34[acc]  = (unsigned char)count;
                    acc                   += add;
                } while (acc < 0x80);
            }
        } else {
            bits_asc       -= 8;
            *p_ch_bits_asc  = bits_asc;

            add = (1 << bits_asc);
            acc = *p_ch_code_asc >> 8;
            do {
                p_work->offs2EB4[acc]  = (unsigned char)count;
                acc                   += add;
            } while (acc < 0x100);
        }
    }
}

//-----------------------------------------------------------------------------
// Skips given number of bits in bit buffer. Result is stored in pWork->bit_buff
// If no data in input buffer, returns true

static int WasteBits(TDcmpStruct *pWork, const unsigned long nBits) {
    // If number of bits required is less than number of (bits in the buffer) ?
    if (nBits <= pWork->extra_bits) {
        pWork->extra_bits  -= nBits;
        pWork->bit_buff   >>= nBits;
        return 0;
    }

    // Load input buffer if necessary
    pWork->bit_buff >>= pWork->extra_bits;
    if (pWork->in_pos == pWork->in_bytes) {
        pWork->in_pos = sizeof(pWork->in_buff);
        if ((pWork->in_bytes = pWork->read_buf((char *)pWork->in_buff, &pWork->in_pos, pWork->param)) == 0)
            return 1;
        pWork->in_pos = 0;
    }

    // Update bit buffer
    pWork->bit_buff    |= (pWork->in_buff[pWork->in_pos++] << 8);
    pWork->bit_buff   >>= (nBits - pWork->extra_bits);
    pWork->extra_bits   = (pWork->extra_bits - nBits) + 8;
    return 0;
}

//-----------------------------------------------------------------------------
// Returns : 0x000 - 0x0FF : One byte from compressed file.
//           0x100 - 0x305 : Copy previous block (0x100 = 1 byte)
//           0x306         : Out of buffer (?)

static unsigned long DecodeLit(TDcmpStruct *pWork) {
    unsigned long value; // Position in buffers

    // Test the current bit in byte buffer. If is not set, simply return the next byte.
    if (pWork->bit_buff & 1) {
        unsigned long nBits; // Number of bits to skip

        // Skip current bit in the buffer
        if (WasteBits(pWork, 1))
            return 0x306;

        // The next bits are position in buffers
        value = pWork->position2[(pWork->bit_buff & 0xFF)];

        // Get number of bits to skip
        if (WasteBits(pWork, pWork->LenBits[value]))
            return 0x306;

        if ((nBits = pWork->ExLenBits[value]) != 0) {
            const unsigned long val2 = pWork->bit_buff & ((1 << nBits) - 1);

            if (WasteBits(pWork, nBits)) {
                if ((value + val2) != 0x10E)
                    return 0x306;
            }
            value = pWork->LenBase[value] + val2;
        }
        return value + 0x100; // Return number of bytes to repeat
    }

    // Waste one bit
    if (WasteBits(pWork, 1))
        return 0x306;

    // If the binary compression type, read 8 bits and return them as one byte.
    if (pWork->ctype == CMP_BINARY) {
        value = pWork->bit_buff & 0xFF;
        if (WasteBits(pWork, 8))
            return 0x306;
        return value;
    }

    // When ASCII compression ...
    if (pWork->bit_buff & 0xFF) {
        value = pWork->offs2C34[pWork->bit_buff & 0xFF];

        if (value == 0xFF) {
            if (pWork->bit_buff & 0x3F) {
                if (WasteBits(pWork, 4))
                    return 0x306;

                value = pWork->offs2D34[pWork->bit_buff & 0xFF];
            } else {
                if (WasteBits(pWork, 6))
                    return 0x306;

                value = pWork->offs2E34[pWork->bit_buff & 0x7F];
            }
        }
    } else {
        if (WasteBits(pWork, 8))
            return 0x306;

        value = pWork->offs2EB4[pWork->bit_buff & 0xFF];
    }

    return WasteBits(pWork, pWork->ChBitsAsc[value]) ? 0x306 : value;
}

//-----------------------------------------------------------------------------
// Retrieves the number of bytes to move back

static unsigned long DecodeDist(TDcmpStruct *pWork, const unsigned long dwLength) {
    unsigned long       pos   = pWork->position1[(pWork->bit_buff & 0xFF)];
    const unsigned long nSkip = pWork->DistBits[pos]; // Number of bits to skip

    // Skip the appropriate number of bits
    if (WasteBits(pWork, nSkip) == 1)
        return 0;

    if (dwLength == 2) {
        pos = (pos << 2) | (pWork->bit_buff & 0x03);

        if (WasteBits(pWork, 2) == 1)
            return 0;
    } else {
        pos = (pos << pWork->dsize_bits) | (pWork->bit_buff & pWork->dsize_mask);

        // Skip the bits
        if (WasteBits(pWork, pWork->dsize_bits) == 1)
            return 0;
    }
    return pos + 1;
}

static unsigned long Expand(TDcmpStruct *pWork) {
    unsigned int  copyBytes; // Number of bytes to copy
    unsigned long oneByte;   // One byte from compressed file
    unsigned long dwResult;

    pWork->outputPos = 0x1000; // Initialize output buffer position

    // If end of data or error, terminate decompress
    while ((dwResult = oneByte = DecodeLit(pWork)) < 0x305) {
        // If one byte is greater than 0x100, means "Repeat n - 0xFE bytes"
        if (oneByte >= 0x100) {
            // ECX
            // EDX
            unsigned long copy_length = oneByte - 0xFE;
            unsigned long move_back;

            // Get length of data to copy
            if ((move_back = DecodeDist(pWork, copy_length)) == 0) {
                dwResult = 0x306;
                break;
            }

            // Target and source pointer
            unsigned char       *target  = &pWork->out_buff[pWork->outputPos];
            const unsigned char *source  = target - move_back;
            pWork->outputPos            += copy_length;

            while (copy_length-- > 0)
                *target++ = *source++;
        } else
            pWork->out_buff[pWork->outputPos++] = (unsigned char)oneByte;

        // If number of extracted bytes has reached 1/2 of output buffer,
        // flush output buffer.
        if (pWork->outputPos >= 0x2000) {
            // Copy decompressed data into user buffer
            copyBytes = 0x1000;
            pWork->write_buf((char *)&pWork->out_buff[0x1000], &copyBytes, pWork->param);

            // If there are some data left, keep them alive
            memcpy(pWork->out_buff, &pWork->out_buff[0x1000], pWork->outputPos - 0x1000);
            pWork->outputPos -= 0x1000;
        }
    }

    copyBytes = pWork->outputPos - 0x1000;
    pWork->write_buf((char *)&pWork->out_buff[0x1000], &copyBytes, pWork->param);
    return dwResult;
}

//-----------------------------------------------------------------------------
// Main exploding function.

unsigned int PKEXPORT explode(unsigned int (*read_buf)(char *buf, unsigned int *size, void *param),
                              void (*write_buf)(char *buf, unsigned int *size, void *param), char *work_buf,
                              void *param) {
    TDcmpStruct *pWork = (TDcmpStruct *)work_buf;

    // Initialize work struct and load compressed data
    pWork->read_buf  = read_buf;
    pWork->write_buf = write_buf;
    pWork->param     = param;
    pWork->in_pos    = sizeof(pWork->in_buff);
    pWork->in_bytes  = pWork->read_buf((char *)pWork->in_buff, &pWork->in_pos, pWork->param);
    if (pWork->in_bytes <= 4)
        return CMP_BAD_DATA;

    pWork->ctype      = pWork->in_buff[0]; // Get the compression type
    pWork->dsize_bits = pWork->in_buff[1]; // Get the dictionary size
    pWork->bit_buff   = pWork->in_buff[2]; // Initialize 16-bit bit buffer
    pWork->extra_bits = 0;                 // Extra (over 8) bits
    pWork->in_pos     = 3;                 // Position in input buffer

    // Test for the valid dictionary size
    if (4 > pWork->dsize_bits || pWork->dsize_bits > 6)
        return CMP_INVALID_DICTSIZE;

    pWork->dsize_mask = 0xFFFF >> (0x10 - pWork->dsize_bits); // Shifted by 'sar' instruction

    if (pWork->ctype != CMP_BINARY) {
        if (pWork->ctype != CMP_ASCII)
            return CMP_INVALID_MODE;

        memcpy(pWork->ChBitsAsc, ChBitsAsc, sizeof(pWork->ChBitsAsc));
        gen_asc_tabs(pWork);
    }

    memcpy(pWork->LenBits, LenBits, sizeof(pWork->LenBits));
    gen_decode_tabs(0x10, pWork->LenBits, LenCode, pWork->position2);
    memcpy(pWork->ExLenBits, ExLenBits, sizeof(pWork->ExLenBits));
    memcpy(pWork->LenBase, LenBase, sizeof(pWork->LenBase));
    memcpy(pWork->DistBits, DistBits, sizeof(pWork->DistBits));
    gen_decode_tabs(0x40, pWork->DistBits, DistCode, pWork->position1);
    if (Expand(pWork) != 0x306)
        return CMP_NO_ERROR;

    return CMP_ABORT;
}

static char copy_right[] = "PKWARE Data Compression Library for Win32\r\n"
                           "Copyright 1989-1995 PKWARE Inc.  All Rights Reserved\r\n"
                           "Patent No. 5,051,745\r\n"
                           "PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.\r\n"
                           "Version 1.11\r\n";

static unsigned long crc_table[] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
    0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
    0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
    0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
    0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
    0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
    0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525,
    0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
    0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
    0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6,
    0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
    0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7,
    0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
    0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330,
    0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

unsigned long PKEXPORT crc32_pk(const char *buffer, const unsigned int *psize, const unsigned long *old_crc) {
    unsigned int  size      = *psize;
    unsigned long crc_value = *old_crc;

    while (size-- != 0) {
        const unsigned long ch   = *buffer++ ^ (char)crc_value;
        crc_value              >>= 8;

        crc_value = crc_table[ch & 0x0FF] ^ crc_value;
    }
    return crc_value;
}
