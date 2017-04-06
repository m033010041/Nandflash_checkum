#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <time.h>

#define MIN_BLOCK_SIZE (0x20000) //2k * 64 bytes/per block

using namespace std;

unsigned int crc32_sum(unsigned int crc, unsigned char const *p, size_t len)
{
    unsigned long circle;

    for(circle = 0; circle < len; circle++)
    {
        crc += *p++;
    }

    return crc;
}

int make_binary_file(FILE *fpin, int size)
{
    FILE *fpout = NULL;
    unsigned int tmp;
    int wsize = 0;
    unsigned char buffer[MIN_BLOCK_SIZE];

    int i=0;

    fpout = fopen("tmp.bin", "wb");

    if(fpout == NULL)
    {
        printf("tmp.bin open failed! \n");
        return -1;
    }

    while(size > 0)
    {
        tmp = (size < MIN_BLOCK_SIZE)? size : MIN_BLOCK_SIZE;

        if(tmp < MIN_BLOCK_SIZE)
            memset(buffer, 0xff, MIN_BLOCK_SIZE);

        tmp = fread(buffer, 1, tmp, fpin);

        if(tmp <= 0)
        {
            printf("read file error\n");
            return -1;
        }

        fwrite(buffer, 1, MIN_BLOCK_SIZE, fpout);
        size = size - tmp;
        wsize = wsize + MIN_BLOCK_SIZE;
        i++;
    }
    fclose(fpout);
    printf("%d %d\n", i, wsize);
    printf("done\n");
    return wsize;
}


int main(int argc, char ** argv)
{
    FILE *fp = NULL;
    FILE *fpout = NULL;
    unsigned int checksum = 0;
    int size = 0, wsize = 0;
    size_t ret;
    int readval;

    if(argc != 2) {
        printf("Check format!!\nHint:./checksum <binary/img file name>\n");
        exit(1);
    }

    fp = fopen(argv[1], "rb");

    if(fp == NULL) {
        printf("Cannot open file %s\n", argv[1]);
        fclose(fp);
        exit(1);
    }


    printf("Check file size ...\n");
    fseek(fp, 0, SEEK_END);
    size = (int)ftell(fp);
    printf("%s size is %d bytes\n", argv[1], size);
    fseek(fp, 0, SEEK_SET);

    printf("Create fill block file ...\n");
    wsize = make_binary_file(fp, size);

    if(wsize <= 0)
    {
        printf("write file error\n");
        exit(1);
    }

    fclose(fp); //close input file, we do not use it anymore

    size = (wsize < (3*MIN_BLOCK_SIZE))? wsize : (3*MIN_BLOCK_SIZE);


    fpout = fopen("tmp.bin", "rb");
    fseek(fpout, (-1*size), SEEK_END); //reset tmp.bin file pointer

    printf("Calculate checksum...\n");
    clock_t start = clock();
    /* Read 3 blocks end of data */
    size = size / sizeof(int);
    for(int i=0; i<size; i++)
    {
        //fseek(fpout, -4, SEEK_CUR);
        fread(&readval, sizeof(int), 1, fpout);
        checksum += readval;
    }
    clock_t end = clock();

    printf("3 block of checksum is 0x%08x\n", checksum);
    cout << "Used " << (end - start)*1.0/CLOCKS_PER_SEC << " sec to calculate." << endl;

    return 0;
}
