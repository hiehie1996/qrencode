#include "myhead.h"
#include "qrencode.h"

#if 1
#define QT_OUT_FILE     "qrpay.bmp"
// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)
#define OUT_FILE_PIXEL_PRESCALER    4       
// Color of bmp pixels  
#define PIXEL_COLOR_R               0        
#define PIXEL_COLOR_G               0
#define PIXEL_COLOR_B               0
#define BI_RGB                      0L




#pragma pack(push, 2) //2字节对齐，不然会出问题
typedef struct
{
    unsigned short bfType;
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long bfOffBits;
} BITMAPFILEHEADER;


typedef struct
{
    unsigned long biSize;
    signed long biWidth;
    signed long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long biCompression;
    unsigned long biSizeImage;
    signed long biXPelsPerMeter;
    signed long biYPelsPerMeter;
    unsigned long biClrUsed;
    unsigned long biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)




int Save_QRImage(char* szSourceSring,char *QrImagePath)
{
    unsigned int unWidth, x, y, l, n, unWidthAdjusted, unDataBytes;
    unsigned char* pRGBData, *pSourceData, *pDestData;
    QRcode* pQRC;
    FILE* f;


    // Compute QRCode
    if  (pQRC = QRcode_encodeString(szSourceSring, 0, QR_ECLEVEL_H, QR_MODE_8, 1))
    {
        //矩阵的维数
        unWidth = pQRC->width;
        unWidthAdjusted = unWidth * OUT_FILE_PIXEL_PRESCALER * 3;
        //每一个维度占的像素的个数（8），每个像素3个字节
        if (unWidthAdjusted % 4)
        {
            unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
        }
        unDataBytes = unWidthAdjusted * unWidth * OUT_FILE_PIXEL_PRESCALER;


        // Allocate pixels buffer
        if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
        {
            printf("Out of memory");
            return -1;
        }


        // Preset to white
        memset(pRGBData, 0xff, unDataBytes);


        // Prepare bmp headers
        BITMAPFILEHEADER kFileHeader;
        kFileHeader.bfType = 0x4d42;  // "BM"
        kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + unDataBytes;
        kFileHeader.bfReserved1 = 0;
        kFileHeader.bfReserved2 = 0;
        kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


        BITMAPINFOHEADER kInfoHeader;
        kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
        kInfoHeader.biWidth = unWidth * OUT_FILE_PIXEL_PRESCALER;
        kInfoHeader.biHeight = -((int)unWidth * OUT_FILE_PIXEL_PRESCALER);
        kInfoHeader.biPlanes = 1;
        kInfoHeader.biBitCount = 24;
        kInfoHeader.biCompression = BI_RGB;
        kInfoHeader.biSizeImage = 0;
        kInfoHeader.biXPelsPerMeter = 0;
        kInfoHeader.biYPelsPerMeter = 0;
        kInfoHeader.biClrUsed = 0;
        kInfoHeader.biClrImportant = 0;


        // Convert QrCode bits to bmp pixels
        pSourceData = pQRC->data;
        for (y = 0; y < unWidth; y++)
        {
            pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
            for (x = 0; x < unWidth; x++)
            {
                if (*pSourceData & 1)
                {
                    for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
                    {
                        for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
                        {
                            *(pDestData + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_B;
                            *(pDestData + 1 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_G;
                            *(pDestData + 2 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_R;
                        }
                    }
                }
                pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
                pSourceData++;
            }
        }


        // Output the bmp file
        f = fopen(QrImagePath,"ab++");
        if (f != NULL)
        {
            fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
            fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
            fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);


            fclose(f);
        }
        else
        {
            printf("Unable to open file\n");
            return -1;
        }


        // Free data
        free(pRGBData);
        QRcode_free(pQRC);
    }
    else
    {
        printf("NULL returned");
        return -1;
    }


    return 0;
}


int main(void)
{
    Save_QRImage("123456789asdhjkfd",QT_OUT_FILE);
}
#endif