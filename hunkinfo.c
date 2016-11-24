/*
 * HUNK INFO - Show the structure of a hunk binary file.
 *
 * (c) 2012 Andreu Santaren <andreu.santaren@gmail.com>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define HUNK_HEADER         0x3F3
#define HUNK_UNIT           0x3E7
#define HUNK_CODE           0x3E9
#define HUNK_DATA           0x3EA
#define HUNK_BSS            0x3EB
#define HUNK_RELOC32        0x3EC
#define HUNK_DREL32         0x3F7
#define HUNK_SYMBOL         0x3F0
#define HUNK_END            0x3F2

unsigned long read32(FILE *fp)
{
    int ch3 = fgetc(fp);
    int ch2 = fgetc(fp);
    int ch1 = fgetc(fp);
    int ch0 = fgetc(fp);

    if (ch0 == EOF || ch1 == EOF || ch2 == EOF || ch3 == EOF)
    {
        printf("Read32 reached end of file unexpectedly\n");
        fclose(fp);
        exit(2);
    }

    unsigned long dat = (unsigned char)ch3 << 24 |
                        (unsigned char)ch2 << 16 |
                        (unsigned char)ch1 << 8 |
                        (unsigned char)ch0;

    return dat;
}

unsigned long read16(FILE *fp)
{
    int ch1 = fgetc(fp);
    int ch0 = fgetc(fp);

    if (ch0 == EOF || ch1 == EOF)
    {
        printf("Read16 reached end of file unexpectedly\n");
        fclose(fp);
        exit(2);
    }

    unsigned long dat = (unsigned char)ch1 << 8 |
                        (unsigned char)ch0;

    return dat;
}

void readData(unsigned long numdata, FILE *fp)
{
    int i;
    for (i = 0 ; i < numdata ; i ++)
    {
        if (fgetc(fp) == EOF)
        {
            printf("ReadData reached end of file unexpectedly\n");
            fclose(fp);
            exit(2);
        }
    }
}

// Format dels Hunks

int blockhunk(FILE *fp)
{
    unsigned long size = read32(fp);
    printf("Size in long words = %lu\n", size);
    readData(size * 4, fp);
    return 0;
}

int bsshunk(FILE *fp)
{
    printf("Allocable memory = %lu\n", read32(fp));
    return 0;
}

int reloc32hunk(FILE *fp)
{
    while (1)
    {
        unsigned long numoffs = read32(fp);
        if (numoffs == 0) return 0;
        printf("Number of Offsets = %lu\n", numoffs);
        unsigned long numhunks = read32(fp);
        printf("Numbers of hunk = %lu\n", numhunks);
        readData(numoffs * 4, fp);
    }
}

int reloc16hunk(FILE *fp)
{
    while (1)
    {
        unsigned long numoffs = read16(fp);
        if (numoffs == 0)
        {
            if (ftell(fp) % 4 != 0) read16(fp);         // 2 bytes de padding
            return 0;
        }
        printf("Number of Offsets = %lu\n", numoffs);
        unsigned long numhunks = read16(fp);
        printf("Numbers of hunk = %lu\n", numhunks);
        readData(numoffs * 2, fp);
    }
}

int symbolhunk(FILE *fp)
{
    unsigned long strsz;

    while ((strsz = read32(fp)) != 0)               // Fins que trobem una string de mida 0
    {
        strsz = strsz + 1;                          // mida de la string en long words
                                                    // el +1 es pk la cadena acaba amb un char null.
        int i;
        for (i = 0 ; i < strsz ; i++)
        {
            int ch0 = fgetc(fp);
            int ch1 = fgetc(fp);
            int ch2 = fgetc(fp);
            int ch3 = fgetc(fp);

            if (ch0 == EOF || ch1 == EOF || ch2 == EOF || ch3 == EOF)
            {
                printf("SymbolHunk reached end of file unexpectedly\n");
                fclose(fp);
                exit(2);
            }

            if (isprint(ch0)) printf("%c", ch0); else printf(" ");
            if (isprint(ch1)) printf("%c", ch1); else printf(" ");
            if (isprint(ch2)) printf("%c", ch2); else printf(" ");
            if (isprint(ch3)) printf("%c", ch3); else printf(" ");
        }
		
		printf("\n");
    }

    //printf("\n");

    return 0;
}

int hunkformat(unsigned long type, unsigned long lwsize, FILE *fp)
{
    printf("Position = %ld\n", ftell(fp) - 4);

    if (type == 0)
    {
        printf("Null Hunk\n");
        fclose(fp);
        exit(2);
    }
    else if (type == HUNK_CODE)
    {
        printf("HUNK_CODE (0x%X)\n", type);
        return blockhunk(fp);
    }
    else if (type == HUNK_DATA)
    {
        printf("HUNK_DATA (0x%X)\n", type);
        return blockhunk(fp);
    }
    else if (type == HUNK_BSS)
    {
        printf("HUNK_BSS (0x%X)\n", type);
        return bsshunk(fp);
    }
    else if (type == HUNK_RELOC32)
    {
        printf("HUNK_RELOC32 (0x%X)\n", type);
        return reloc32hunk(fp);
    }
    else if (type == HUNK_SYMBOL)
    {
        printf("HUNK_SYMBOL (0x%X)\n", type);
        return symbolhunk(fp);
    }
    else if (type == HUNK_DREL32)
    {
        printf("HUNK_DREL32 (0x%X)\n", type);
        return reloc16hunk(fp);
    }
    else if (type == HUNK_END)
    {
        printf("HUNK_END (0x%X)\n", type);
        return 1;
    }
    else
    {
        printf("UNKNOWN HUNK TYPE (0x%X)\n", type);
        fclose(fp);
        exit(2);
    }

    return 0;
}

unsigned long hunksize[20];                             // llista de mides de hunks, 20 es de sobres

int main(int argc, char **argv)
{
    printf("\nHunkInfo v1.0 (c) 2012 Andreu Santaren Llop");
    printf("\n                  andreu.santaren@gmail.com\n\n");

    if (argc != 2)
    {
        printf("Usage: %s file\n\n", argv[0]);
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("Error openig file %s\n", argv[1]);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);                             // Mida del arxiu
    fseek(fp, 0, SEEK_SET);

    unsigned long magic = read32(fp);                   // Magic Cookie
    unsigned long string = read32(fp);                  // String ?? Sempre val zero

    if (magic == HUNK_UNIT)
    {
        printf("HUNK_UNIT FOUND!! Please send me this file to: andreu.santaren@gmail.com\n");
        fclose(fp);
        exit(2);
    }

    if (magic != HUNK_HEADER )
    {
        printf("Incorrect Hunk Magic Cookie\n");
        fclose(fp);
        exit(1);
    }

    if (string != 0)
    {
        printf("HEADER STRING IS NOT NULL!! Please send me this file to: andreu.santaren@gmail.com\n");
        fclose(fp);
        exit(2);
    }

    printf("-------------------------------------------\n");
    printf("HUNK HEADER :\n");
    printf("-------------------------------------------\n");

    printf("File size = %ld bytes\n", fsize);

    unsigned long numhunks = read32(fp);                // obtenim numero de hunks

    printf("Number of hunks = %lu\n", numhunks);

    printf("Number of first hunk = %lu\n", read32(fp)); // Primer hunk num
    printf("Number of last hunk = %lu\n", read32(fp));  // Ultim hunk num

    int i = 0;
    for (i = 0 ; i < numhunks ; i ++)                   // llegim la taula de hunksize, en long words (4 bytes)
    {
        hunksize[i] = read32(fp); 
        printf("Size of %d hunk = %lu\n", i, hunksize[i]);
    }

    printf("-------------------------------------------\n");

    i = 0;

    printf("HUNK NUMBER %d :\n", i);
    printf("-------------------------------------------\n");

    while (i < numhunks)                   // recorrem els hunks
    {
        unsigned long hunktype = read32(fp);

        if (hunkformat(hunktype, hunksize[i], fp))
        {
            i ++;

            printf("-------------------------------------------\n");
            printf("HUNK NUMBER %d :\n", i);
        }

        printf("-------------------------------------------\n");
    }

    printf("Happy end at file position = %ld\n\n", ftell(fp));
    fclose(fp);

    return 0;
}
