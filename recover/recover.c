#include <stdio.h>
#include <stdint.h>
#include <cs50.h>
#include <string.h>

typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint16_t WORD;

/*#define bool int
#define true 1
#define false 0*/


typedef struct{
    BYTE header[4];
    BYTE extra[508];
}__attribute__((__packed__)) 
FAT;

bool isSignature(BYTE buffer[]);

int main (int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr,"Usage: ./recover file");
        return 1;
    }
    
    char *rawFile = argv[1];
    
    FILE *rawptr = fopen(rawFile, "r");
    
    if (rawptr == NULL){
        fprintf(stderr, "Unable to open %s", rawFile);
        return 2;
    }
    
    bool eof = false;
    int imgCount = 0; //number of images produced
    char filename[8];
    FILE *outptr;
    
    while (eof == false){
        
        FAT fatBlock;
        int numCopied = fread(&fatBlock, sizeof(FAT), 1, rawptr);
        if (numCopied < 1)
        {
            eof = true;
            break;
        }
        
        if (isSignature(fatBlock.header))
        {
            if (imgCount != 0){
                fclose(outptr);   
            }
            sprintf(filename, "%03i.jpg", imgCount);
            outptr = fopen(filename, "w");
            imgCount++;
        }
        
        if (imgCount != 0){
            fwrite(&fatBlock, 512, 1, outptr);
        }
        
    }
    
    if (imgCount != 0)
    {
        fclose(outptr);
    }
    
}

bool isSignature(BYTE buffer[]){
    if (buffer[0] == 0xff &&
    buffer[1] == 0xd8 &&
    buffer[2] == 0xff)
    {
        return true;
    }
    return false;
    
}