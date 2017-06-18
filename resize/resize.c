/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize factor infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    
    //resize factor
    float f = atof(argv[1]);
    
    if (f <= 0 || f > 100){
        fprintf(stderr, "Resize factor must be in the range (0,100].");
        return 1;
    }

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 1;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 1;
    }
    
    

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        //printf("bfType: %h\n bfOffBits: %i")
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 1;
    }
    
    //Save old and new heights/widths
    LONG oldBiHeight = bi.biHeight;
    LONG oldBiWidth = bi.biWidth;
    LONG newBiWidth = oldBiWidth * f;
    LONG newBiHeight = oldBiHeight * f;

    // determine padding for scanlines
    int oldPadding = (4 - (oldBiWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int newPadding = (4 - (newBiWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    
    int intf = (int) f;
    float floatf = f - intf;
    
    bi.biHeight = newBiHeight;
    bi.biWidth = newBiWidth;
    bi.biSizeImage = abs(bi.biHeight) * bi.biWidth * 3 + newPadding * abs(newBiHeight);
    
    //Update the filesize
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    
    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);
    
    
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(oldBiHeight); i < biHeight; i++)
    {
        //Number of times we repeat a row
        int rowRepeats = intf + ((int) ((i+1) * floatf)) % 2; 
        
        //If we are skipping a row, move the file pointer 1 row ahead
        if (rowRepeats == 0){
            fseek(inptr, oldBiWidth * 3 + oldPadding, SEEK_CUR);
        }
        
        //Loop over the number of repeated rows
        for (int rowIter = 0; rowIter < rowRepeats; rowIter++)
        {
            //Move the file pointer back 1 row if it is not the first time cycling through the for loop
            if (rowIter != 0)
            {
                fseek(inptr, - oldBiWidth * 3 - oldPadding, SEEK_CUR);
            }
            
            
            for (int j = 0; j < oldBiWidth; j++)
            {
                //Number of times the pixel is repeated
                int pixRepeats = intf + ((int)((j+1) * floatf)) % 2;
                
                //Move file pointer over 3 bytes (1 pixel) if we are skipping a pixel
                /*if (pixRepeats == 0){
                    fseek(inptr, 3, SEEK_CUR);
                }*/
                
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                
                for (int pixIter = 0; pixIter < pixRepeats; pixIter++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
                
            }
            // skip over padding in the input file
            fseek(inptr, oldPadding, SEEK_CUR);
    
            // Add padding to the output file
            for (int k = 0; k < newPadding; k++)
            {
                fputc(0x00, outptr);
            }
        }
        
        

        
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}