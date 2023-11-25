/*
*   
*   MGS HD and Master Collection Save Decryptor - Updated for Master Collection by LiquidPlazmid
*
*   This tool is based on the MGS PS3 Save Decrypter - (c) 2021 by Bucanero and the original XBOX MGS 2/3 - SecFixers by Philymaster
*
*/

#include "asprintf.h"
#include "iofile.c"

#define MGS2_KEY            "2MetalOfSolidSonsLibertyGear"
#define MGS3_KEY            "3MetalSolidSnakeEaterGear"
#define MGS2_ALPHABET       "ghijklmn01234567opqrstuvEFGHIJKL89abcdefUVWXYZ_.wxyzABCDMNOPQRST"
#define MGS3_ALPHABET       "ghijklmn01234567opqrstuvEFGHIJKL89abcdefUVWXYZ+-wxyzABCDMNOPQRST"

#define CRC_POLY            0xEDB88320

int vasprintf(char **strp, const char *format, va_list ap)
{
    int len = vscprintf(format, ap);
    if (len == -1)
        return -1;
    char *str = (char*)malloc((size_t) len + 1);
    if (!str)
        return -1;
    int retval = vsnprintf(str, len + 1, format, ap);
    if (retval == -1) {
        free(str);
        return -1;
    }
    *strp = str;
    return retval;
}

int asprintf(char **strp, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(strp, format, ap);
    va_end(ap);
    return retval;
}

int CalculateCRC32(const u8* data, int size)
{
    int crc = -1;

    while (size--)
    {
        crc = crc ^ *data++ << 24 >> 24;
        for (int j = 0; j < 8; j++)
        {
            int num = crc & 1;
            crc = crc >> 1 & 0x7fffffff;
            if (num != 0)
            {
                crc ^= CRC_POLY;
            }
        }
    }

    return ~crc;
}

u32 CalculateCRC32Switch(const u8* data, int size)
{
    u32 crc = -1;

    while (size--)
    {
        crc = crc ^ *data++;
        for (int j = 0; j < 8; j++)
        {
            int num = crc & 1;
            crc = crc >> 1;
            if (num != 0)
            {
                crc ^= CRC_POLY;
            }
        }
    }

    return crc;
}

void crc32_fill(uint32_t *table, uint32_t poly)
{
    uint8_t index=0, z;
    do
    {
        table[index]=index;
        for(z=8; z; z--)
        	table[index] = (table[index] & 1) ? (table[index] >> 1)^poly : table[index] >> 1;
    } while(++index);
}

u32 crc32(u8* input, u32 len)
{
	u32 crc_table[256];
	u32 crc = 0xFFFFFFFF;

	crc32_fill(crc_table, CRC_POLY);

	for (int i = 0; i < len; i++)
		crc = (crc >> 8 ^ crc_table[((crc ^ (u32)input[i]) & 0xFF)]);

	return ~crc;
}

void Decrypt(const char* key, u8* data, int size)
{
    int keylen = (key[0] ? strlen(key) : 1);

    for (int i = size - 1; i > 0; i--)
        data[i] -= (key[i % keylen] + data[i-1]);
    
    data[0] -= key[0];
    return;
}

void Encrypt(const char* key, u8* data, int size)
{
    int keylen = (key[0] ? strlen(key) : 1);
    data[0] += key[0];

    for (int i = 1; i < size; i++)
        data[i] += (key[i % keylen] + data[i-1]);

    return;
}

void EncodeBase64(u8* data, const char* chars)
{
    int i, j, k;
    u8 tmpArray[28];

    data[20] = 0;
    for (j = 0; j < 20; j++)
        data[20] ^= data[j];

    for (i = 0, j = 0, k = 0; j < 28; j++)
    {
        if (k == 0 || k == 1)
            tmpArray[j] = (u8)(data[i] >> (2-k));

        else if (k == 2)
            tmpArray[j] = (u8)(data[i] & 63);

        else if (k <= 7)
            tmpArray[j] = (u8)(data[i + 1] >> (10-k)) | ((data[i] & ((1 << (8-k)) - 1)) << (k-2));

        k += 6;
        if (k >= 8)
        {
            k -= 8;
            i++;
        }
    }

    data[0] = 68;

    for (i = 0, j = 0; i < 28; j += 28, i += 4)
    {
        data[i + 1] = (u8)chars[(tmpArray[i] + j) & 63];
        data[i + 2] = (u8)chars[(tmpArray[i + 1] + j +  7) & 63];
        data[i + 3] = (u8)chars[(tmpArray[i + 2] + j + 14) & 63];
        data[i + 4] = (u8)chars[(tmpArray[i + 3] + j + 21) & 63];
    }

    return;
}

void DecodeBase64(u8* data, const char* str)
{
    int i, j, k, m;
    u8 b64_table[0x100];
    u8 tmpArray[0x20];

    memset(tmpArray, 0, sizeof(tmpArray));
    memset(b64_table, 0xFF, sizeof(b64_table));

    for (k = 0; k < 64; k++)
        b64_table[str[k]] = (u8)k;

    for (j = 0, m = 0; j < 196; j += 7, m++)
    {
        k = b64_table[data[1 + m]];

        if (k == 0xff)
            return;

        data[m] = (u8)((k - j) & 63);
    }

    for (j = 0, k = 0, m = 0; m < 21; m++)
    {
        if (j <= 5)
            tmpArray[m] = (u8)(data[k] & 63 >> (j & 31)) << ((2 + j) & 31);

        i = ~j + 7;
        j = 0;
        k++;

        if (i < 2)
        {
            tmpArray[m] |= (u8)(data[k] << ((~i + 3) & 31));
            i += 6;
            k++;
        }
        i -= 2;

        if (i == 0)
        {
            tmpArray[m] |= (u8)(data[k]);
            k++;
            j = 0;
        }
        else if (i <= 5)
        {
            j = 6 - i;
            tmpArray[m] |= (u8)(data[k] >> (i & 31));
        }
    }
    memcpy(data, tmpArray, sizeof(tmpArray));

    return;
}

void endian_swap_data_of_integers(u8* data, u32 size)
{
    int num_of_integers = size / sizeof(u32);

    for (int i = 0; i < size; i += 4)
    { 
        u8 temp = data[i];
        data[i] = data[i + 3];
        data[i + 3] = temp;
        temp = data[i + 1];
        data[i + 1] = data[i + 2];
        data[i + 2] = temp;
    }
}

void mgs2_decrypt_data(u8* data, u32 size)
{
	printf("[*] MGS2 Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS2_KEY, data + 4, 0x71a6);

    // Decrypt Layer 2
    Decrypt("", data + 4, 0x1598);
    Decrypt("", data + 0x15aa, 0x1c00);
    Decrypt("", data + 0x31aa, 0x4000);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mgs2_encrypt_data(u8* data, u32 size)
{
	printf("[*] MGS2 Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = ES32(CalculateCRC32(data + 4, 0x1598) ^ CalculateCRC32(data + 0x15aa, 0x1c00));
    memcpy(data + 0x15a6L, &crc, sizeof(u32));

    // Encrypt Layer 2
    Encrypt("", data + 4, 0x1598);
    Encrypt("", data + 0x15aa, 0x1c00);
    Encrypt("", data + 0x31aa, 0x4000);

    crc = crc32(data + 4, 0x71a6);
    Encrypt(MGS2_KEY, data + 4, 0x71a6);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void mgs2_mc_decrypt_data(u8* data, u32 size)
{
	printf("[*] MGS2 MC Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS2_KEY, data + 4, 0x72a6);

    // Decrypt Layer 2
    Decrypt("", data + 4, 0x1598);
    Decrypt("", data + 0x15aa, 0x1c00);
    Decrypt("", data + 0x31aa, 0x4100);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mgs2_mc_encrypt_data_pc(u8* data, u32 size)
{
	printf("[*] MGS2 MC Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);
    u32 crc = ES32(CalculateCRC32(data + 4, 0x1598) ^ CalculateCRC32(data + 0x15aa, 0x1c00));
    u32 endian_swapped_crc = ((crc>>24)&0xff) | ((crc<<8)&0xff0000) | ((crc>>8)&0xff00) | ((crc<<24)&0xff000000); 
    memcpy(data + 0x15a6L, &endian_swapped_crc, sizeof(u32));

    // Encrypt Layer 2
    Encrypt("", data + 4, 0x1598);
    Encrypt("", data + 0x15aa, 0x1c00);
    Encrypt("", data + 0x31aa, 0x4100);

    crc = crc32(data + 4, 0x72a6);
    Encrypt(MGS2_KEY, data + 4, 0x72a6);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

u32 mgs2_mc_encrypt_data_switch(u8* data, u32 size)
{
	printf("[*] MGS2 MC Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = ES32(CalculateCRC32Switch(data + 4, 0x1598) ^ CalculateCRC32Switch(data + 0x15aa, 0x1c00));    
    u32 endian_swapped_crc = ((crc>>24)&0xff) | ((crc<<8)&0xff0000) | ((crc>>8)&0xff00) | ((crc<<24)&0xff000000); 
    memcpy(data + 0x15a6L, &endian_swapped_crc, sizeof(u32));

    // Encrypt Layer 2
    Encrypt("", data + 4, 0x1598);
    Encrypt("", data + 0x15aa, 0x1c00);
    Encrypt("", data + 0x31aa, 0x4100);

    crc = crc32(data + 4, 0x72a6);
    Encrypt(MGS2_KEY, data + 4, 0x72a6);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void mgs3_decrypt_data(u8* data, u32 size)
{
	printf("[*] MGS3 Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS3_KEY, data, 0x497c);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mgs3_encrypt_data(u8* data, u32 size)
{
	printf("[*] MGS3 Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = crc32(data, 0x497c);
    Encrypt(MGS3_KEY, data, 0x497c);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void mgs3_mc_decrypt_data(u8* data, u32 size)
{
	printf("[*] MGS3 MC Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS3_KEY, data, 0x4A7c);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mgs3_mc_encrypt_data(u8* data, u32 size)
{
	printf("[*] MGS3 MC Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = crc32(data, 0x4A7c);
    Encrypt(MGS3_KEY, data, 0x4A7c);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void mg2_decrypt_data(u8* data, u32 size)
{
	printf("[*] MG2 Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS3_KEY, data, 0x158);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mg2_encrypt_data(u8* data, u32 size)
{
	printf("[*] MG2 Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = crc32(data, 0x158);
    Encrypt(MGS3_KEY, data, 0x158);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void mg1_decrypt_data(u8* data, u32 size)
{
	printf("[*] MG1 Total Decrypted Size Is 0x%X (%d bytes)\n", size, size);

    Decrypt(MGS3_KEY, data, 0xCB);

	printf("[*] Decrypted File Successfully!\n\n");
	return;
}

u32 mg1_encrypt_data(u8* data, u32 size)
{
	printf("[*] MG1 Total Encrypted Size Is 0x%X (%d bytes)\n", size, size);

    u32 crc = crc32(data, 0xCB);
    Encrypt(MGS3_KEY, data, 0xCB);

    printf("[*] New Checksum: %08X\n", crc);
	printf("[*] Encrypted File Successfully!\n\n");
	return (ES32(crc));
}

void print_usage(const char* argv0)
{
	printf("USAGE: %s [operation] [game] [platform] filename\n\n", argv0);
	printf("OPERATIONS     Explanation:\n");
	printf(" -d            Decrypt File\n");
	printf(" -e            Encrypt File\n");
    printf(" -c            Convert File*\n");
    printf("GAMES          Explanation:\n");
    printf(" -0            MG1 File\n");
    printf(" -1            MG2 File\n");
    printf(" -2            MGS2 File\n");
	printf(" -3            MGS3 File\n");
    printf("PLATFORMS      Explanation:\n");
    printf(" -h            HD Collection File\n");
	printf(" -p            Master Collection File (PC)\n");
    printf(" -s            Master Collection File (Switch)\n");
    printf("* Save conversion behavior differs between games\n");
    printf("* MG1 & MG2 saves can be converted with the conversion operation from HD to Master Collection and back by copying\n");
    printf("* the contents of the save into an existing save file on the target platform and running the conversion operation.\n");
    printf("* Going from MC to HD requires MC save data to be copied into an HD DATA.BIN file and a MASTER.BIN file to be in the same directory.\n");
    printf("* Going from HD to MC requires HD save data to be copied into a MC save file, which will need to be renamed after the conversion.\n");
    printf("* MGS2 saves are converted between PC and Switch with the conversion operation.\n");
    printf("* Save will be converted to the specified platform and will be assumed to be from the opposite platform.\n");
    printf("* Attempting to use the conversion flag with a platform that matched the platform of the save will produce an invalid save file.\n");
	return;
}

int main(int argc, char **argv)
{
	size_t len, temp_len;
	u8* data;
	char *opt1, *opt2, *opt3, *bak, type;
    int is_decrypt = 0, is_encrypt = 0, is_convert = 0, is_mg = 0, is_mg2 = 0, is_mgs2 = 0, is_mgs3 = 0, is_hd = 0, is_pc = 0, is_switch = 0;

	printf("\nMGS HD and Master Collection Save Decryptor 1.2.0 - (c) 2023 by LiquidPlazmid\n\n");
	
	if (--argc < 4)
	{
		print_usage(argv[0]);
		return -1;
	}
	
	opt1 = argv[1];
	if (*opt1++ != '-' || (*opt1 != 'd' && *opt1 != 'e' && *opt1 != 'c' && *opt1 != '0' && *opt1 != '1' && *opt1 != '2' && *opt1 != '3' && *opt1 != 'h' && *opt1 != 'p' && *opt1 != 's'))
	{
		print_usage(argv[0]);
		return -1;
	}

    opt2 = argv[2];
	if (*opt2++ != '-' || (*opt2 != 'd' && *opt2 != 'e' && *opt2 != 'c' && *opt2 != '0' && *opt2 != '1' && *opt2 != '2' && *opt2 != '3' && *opt2 != 'h' && *opt2 != 'p' && *opt2 != 's'))
	{
		print_usage(argv[0]);
		return -1;
	}

    opt3 = argv[3];
	if (*opt3++ != '-' || (*opt3 != 'd' && *opt3 != 'e' && *opt3 != 'c' && *opt3 != '0' && *opt3 != '1' && *opt3 != '2' && *opt3 != '3' && *opt3 != 'h' && *opt3 != 'p' && *opt3 != 's'))
	{
		print_usage(argv[0]);
		return -1;
	}

    switch (*opt1) {
        case 'd':
            is_decrypt = 1;
            break;
        case 'e':
            is_encrypt = 1;
            break;
        case 'c':
            is_convert = 1;
            break;
        case '0':
            is_mg = 1;
            break;
        case '1':
            is_mg2 = 1;
            break;
        case '2':
            is_mgs2 = 1;
            break;
        case '3':
            is_mgs3 = 1;
            break;
        case 'h':
            is_hd = 1;
            break;
        case 'p':
            is_pc = 1;
            break;
        case 's':
            is_switch = 1;
            break;
        default:
            break;
    }

    switch (*opt2) {
        case 'd':
            is_decrypt = 1;
            break;
        case 'e':
            is_encrypt = 1;
            break;
        case 'c':
            is_convert = 1;
            break;
        case '0':
            is_mg = 1;
            break;
        case '1':
            is_mg2 = 1;
            break;
        case '2':
            is_mgs2 = 1;
            break;
        case '3':
            is_mgs3 = 1;
            break;
        case 'h':
            is_hd = 1;
            break;
        case 'p':
            is_pc = 1;
            break;
        case 's':
            is_switch = 1;
            break;
        default:
            break;
    }

    switch (*opt3) {
        case 'd':
            is_decrypt = 1;
            break;
        case 'e':
            is_encrypt = 1;
            break;
        case 'c':
            is_convert = 1;
            break;
        case '0':
            is_mg = 1;
            break;
        case '1':
            is_mg2 = 1;
            break;
        case '2':
            is_mgs2 = 1;
            break;
        case '3':
            is_mgs3 = 1;
            break;
        case 'h':
            is_hd = 1;
            break;
        case 'p':
            is_pc = 1;
            break;
        case 's':
            is_switch = 1;
            break;
        default:
            break;
    }

	if (read_buffer(argv[4], &data, &len) != 0)
	{
		printf("[*] Could Not Access The File (%s)\n", argv[4]);
		return -1;
	}

	// Save a file backup
	asprintf(&bak, "%s.bak", argv[4]);
	write_buffer(bak, data, len);

	if (is_decrypt && is_mgs2 && is_hd)
	{
		mgs2_decrypt_data(data, len);
		write_buffer(argv[4], data, len);
	    free(data);

		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);

		DecodeBase64(data, MGS2_ALPHABET);
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Decoded\n", "MASTER.BIN");
	}
    else if (is_decrypt && is_mgs2 && (is_pc || is_switch))
	{
		mgs2_mc_decrypt_data(data, len);
		write_buffer(argv[4], data, len);
	    free(data);
	}
	else if (is_encrypt && is_mgs2 && is_hd)
	{
		u32 crc = mgs2_encrypt_data(data, len);
		write_buffer(argv[4], data, len);
	    free(data);
	
		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);
	
	    memcpy(data + 16, &crc, sizeof(u32));
	    EncodeBase64(data, MGS2_ALPHABET);
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Encoded\n", "MASTER.BIN");
	}
    else if (is_encrypt && is_mgs2 && (is_pc || is_switch))
	{
		u32 crc;
        if (is_pc)
            crc = mgs2_mc_encrypt_data_pc(data, len);
        else
            crc = mgs2_mc_encrypt_data_switch(data, len);

		write_buffer(argv[4], data, len);
	    free(data);

        u8* filename = argv[4];
        size_t filename_length = strlen(filename);
        printf("Old filename: %s, Length: %zu\n", filename, filename_length);
	
		if (filename_length != 29)
		{
			printf("[*] Expected filename length of 29. File cannot be used as is if modifications were made.\n");
			return -1;
		}

        DecodeBase64(filename, MGS2_ALPHABET);
	    memcpy(filename + 16, &crc, sizeof(u32));
	    EncodeBase64(filename, MGS2_ALPHABET);
        printf("New filename: %s. Please rename encrypted file to %s\n", filename, filename);
        free(filename);
	}
	else if (is_decrypt && (is_mgs3 || is_mg || is_mg2) && is_hd)
	{
		if (is_mgs3)
        {
            mgs3_mc_decrypt_data(data, len);
        }
        else if (is_mg2)
        {
            mg2_decrypt_data(data, len);
        }
        else 
        {
            mg1_decrypt_data(data, len);
        }

		write_buffer(argv[4], data, len);
	    free(data);

		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);

		DecodeBase64(data, MGS3_ALPHABET);
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Decoded\n", "MASTER.BIN");
	}
    else if (is_decrypt && (is_mgs3 || is_mg || is_mg2) && (is_pc || is_switch))
	{
        if (is_mgs3)
        {
            mgs3_mc_decrypt_data(data, len);
        }
        else if (is_mg2)
        {
            mg2_decrypt_data(data, len);
        }
        else 
        {
            mg1_decrypt_data(data, len);
        }
		
		write_buffer(argv[4], data, len);
	    free(data);
	}
	else if (is_encrypt && (is_mgs3 || is_mg || is_mg2) && is_hd)
	{
		u32 crc = 0;
        if (is_mgs3) 
        {
            crc = mgs3_encrypt_data(data, len);
        }
        else if (is_mg2)
        {
            crc = mg2_encrypt_data(data, len);
        }
        else
        {
            crc = mg1_encrypt_data(data, len);
        }

		write_buffer(argv[4], data, len);
	    free(data);
	
		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);

	    memcpy(data + 8, &crc, sizeof(u32));
	    memcpy(data + 16, &crc, sizeof(u32));
		EncodeBase64(data, MGS3_ALPHABET);
		data[0] = 0x5F;
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Encoded\n", "MASTER.BIN");
	}
    else if (is_encrypt && (is_mgs3 || is_mg || is_mg2) && (is_pc || is_switch))
	{
		u32 crc = 0;
        if (is_mgs3) 
        {
            crc = mgs3_mc_encrypt_data(data, len);
        }
        else if (is_mg2)
        {
            crc = mg2_encrypt_data(data, len);
        }
        else
        {
            crc = mg1_encrypt_data(data, len);
        }
        
		write_buffer(argv[4], data, len);
	    free(data);

        u8* filename = argv[4];
        size_t filename_length = strlen(filename);
        printf("Old filename: %s, Length: %zu\n", filename, filename_length);

		if (filename_length != 29)
		{
			printf("[*] Expected filename length of 29. File cannot be used as is if modifications were made.\n");
			return -1;
		}

        DecodeBase64(filename, MGS3_ALPHABET);
	    memcpy(filename + 8, &crc, sizeof(u32));
	    memcpy(filename + 16, &crc, sizeof(u32));
		EncodeBase64(filename, MGS3_ALPHABET);
		filename[0] = 0x5F;
        printf("New filename: %s. Please rename encrypted file to %s\n", filename, filename);
        free(filename);
	}
    else if (is_convert && is_mg2 && (is_pc || is_switch))
    {
        printf("Converting MG2 HD save to Master Collection save\n");

        mg2_decrypt_data(data, len);
        endian_swap_data_of_integers(data, len);
        u32 crc = mg2_encrypt_data(data, len);

		write_buffer(argv[4], data, len);
	    free(data);
	
		u8* filename = argv[4];
        size_t filename_length = strlen(filename);
        printf("Old filename: %s, Length: %zu\n", filename, filename_length);

		if (filename_length != 29)
		{
			printf("[*] Expected filename length of 29. Please copy HD Collection save into an existing Master Collection save and try again.\n");
			return -1;
		}

        DecodeBase64(filename, MGS3_ALPHABET);
	    memcpy(filename + 8, &crc, sizeof(u32));
	    memcpy(filename + 16, &crc, sizeof(u32));
		EncodeBase64(filename, MGS3_ALPHABET);
		filename[0] = 0x5F;
        printf("New filename: %s. Please rename converted file to %s\n", filename, filename);
        free(filename);
    }
    else if (is_convert && is_mg && (is_pc || is_switch))
    {
        printf("Converting MG1 HD save to Master Collection save\n");

        mg1_decrypt_data(data, len);
        u32 crc = mg1_encrypt_data(data, len);

		write_buffer(argv[4], data, len);
	    free(data);
	
		u8* filename = argv[4];
        size_t filename_length = strlen(filename);
        printf("Old filename: %s, Length: %zu\n", filename, filename_length);

		if (filename_length != 29)
		{
			printf("[*] Expected filename length of 29. Please copy HD Collection save into an existing Master Collection save and try again.\n");
			return -1;
		}

        DecodeBase64(filename, MGS3_ALPHABET);
	    memcpy(filename + 8, &crc, sizeof(u32));
	    memcpy(filename + 16, &crc, sizeof(u32));
		EncodeBase64(filename, MGS3_ALPHABET);
		filename[0] = 0x5F;
        printf("New filename: %s. Please rename converted file to %s\n", filename, filename);
        free(filename);
    }
    else if (is_convert && is_mg2 && is_hd)
    {
        printf("Converting MG2 Master Collection save to HD Collection save\n");

        mg2_decrypt_data(data, len);
        endian_swap_data_of_integers(data, len);
        u32 crc = mg2_encrypt_data(data, len);

		write_buffer(argv[4], data, len);
	    free(data);
	
		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
            printf("Please copy Master Collection save into existing HD Collection DATA.BIN save and have a valid HD Collection MASTER.BIN filein the same directory.\n");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);

	    memcpy(data + 8, &crc, sizeof(u32));
	    memcpy(data + 16, &crc, sizeof(u32));
		EncodeBase64(data, MGS3_ALPHABET);
		data[0] = 0x5F;
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Encoded\n", "MASTER.BIN");
    }
    else if (is_convert && is_mg && is_hd)
    {
        printf("Converting MG Master Collection save to HD Collection save\n");

        mg1_decrypt_data(data, len);
        u32 crc = mg1_encrypt_data(data, len);

		write_buffer(argv[4], data, len);
	    free(data);
	
		if (read_buffer("MASTER.BIN", &data, &len) != 0)
		{
			printf("[*] Could Not Access The File (%s)\n", "MASTER.BIN");
            printf("Please copy Master Collection save into existing HD Collection DATA.BIN save and have a valid HD Collection MASTER.BIN filein the same directory.\n");
			return -1;
		}

		write_buffer("MASTER.BIN.bak", data, len);

	    memcpy(data + 8, &crc, sizeof(u32));
	    memcpy(data + 16, &crc, sizeof(u32));
		EncodeBase64(data, MGS3_ALPHABET);
		data[0] = 0x5F;
		write_buffer("MASTER.BIN", data, len);

		printf("[*] %s File Encoded\n", "MASTER.BIN");
    }
    else if (is_convert && is_mgs2 && (is_pc || is_switch))
	{
        mgs2_mc_decrypt_data(data, len);

		u32 crc;
        if (is_pc)
            crc = mgs2_mc_encrypt_data_pc(data, len);
        else
            crc = mgs2_mc_encrypt_data_switch(data, len);

		write_buffer(argv[4], data, len);
	    free(data);

        u8* filename = argv[4];
        size_t filename_length = strlen(filename);
        printf("Old filename: %s, Length: %zu\n", filename, filename_length);
	
		if (filename_length != 29)
		{
			printf("[*] Expected filename length of 29. File cannot be used as is if modifications were made.\n");
			return -1;
		}

        DecodeBase64(filename, MGS2_ALPHABET);
	    memcpy(filename + 16, &crc, sizeof(u32));
	    EncodeBase64(filename, MGS2_ALPHABET);
        printf("New filename: %s. Please rename encrypted file to %s\n", filename, filename);
        free(filename);
	}
    else 
    {
        print_usage(argv[0]);
    }

	free(bak);
	free(data);
	
	return 0;
}
