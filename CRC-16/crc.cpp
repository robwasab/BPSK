#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define POLY 0x1021

uint16_t * compute_crc_table()
{
    int n;
    int k;
    uint16_t * table;
    uint32_t reg;

    table = (uint16_t *) malloc(sizeof(uint16_t) * 256);

    for (k = 0; k < 256; k++)
    {
        reg = k << 8;

        for (n = 7; n >= 0; n--)
        {
            reg <<= 1;
            if (reg & (1 << 16))
            {
                reg ^= POLY;
            }
        }
        table[k] = (uint16_t) (reg & 0xffff);
    }

    return table;
}

uint16_t crc_16(const uint16_t table[], const uint8_t msg[], size_t len)
{
    size_t k;
	uint16_t reg = 0;

    for (k = 0; k < len; k++)
    {
        reg = (reg << 8 | msg[k]) ^ table[0xff & (reg >> 8)];
    }

    for (k = 0; k < 2; k++)
    {
        reg = (reg << 8) ^ table[0xff & (reg >> 8)];
    }
    return reg;
}

#ifdef __MAIN__
int main(void)
{
    int k;
    uint16_t * table;
    uint16_t crc;

    table = compute_crc_table();

    /*
    for (k = 0; k < 256; k++)
    {
        printf("[%d]: %x\n", k, table[k]);
    }
    */

    //uint8_t msg[] = {0x01, 0x10, 0x21, 0xff};
    uint8_t msg[] = {0xde, 0xad, 0xbe, 0xef};

    crc = crc_16(table, msg, sizeof(msg));

    printf("crc: %x\n", crc);

    free(table);

    return 0;
}
#endif

