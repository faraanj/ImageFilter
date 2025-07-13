#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

// Read BMP header and metadata from stdin
Bitmap *read_header() {
    Bitmap *bitmap = malloc(sizeof(Bitmap));
 
    unsigned char data7[BMP_FILE_SIZE_OFFSET];
    fread(data7, BMP_FILE_SIZE_OFFSET, 1, stdin);
 
    unsigned char data8[4];
    fread(data8, sizeof(int),1,stdin);
    memcpy(&(bitmap->fileSize),data8,sizeof(int));
 
    unsigned char data1[BMP_HEADER_SIZE_OFFSET-BMP_FILE_SIZE_OFFSET-4];
    fread(data1, BMP_HEADER_SIZE_OFFSET-BMP_FILE_SIZE_OFFSET-4, 1, stdin);
 
    unsigned char data2[4];
    fread(data2, sizeof(int), 1, stdin);
    memcpy(&(bitmap->headerSize),data2,sizeof(int));
 
    unsigned char data3[BMP_WIDTH_OFFSET-BMP_HEADER_SIZE_OFFSET-4];
    fread(data3, BMP_WIDTH_OFFSET-BMP_HEADER_SIZE_OFFSET-4,1,stdin);
 
    unsigned char data4[4];
    fread(data4, sizeof(int),1,stdin);
    memcpy(&(bitmap->width),data4,sizeof(int));
 
    unsigned char data5[4];
    fread(data5,sizeof(int),1,stdin);
    memcpy(&(bitmap->height), data5, sizeof(int));
 
    unsigned char data6[bitmap->headerSize - BMP_HEIGHT_OFFSET - 4];
    fread(data6,bitmap->headerSize - BMP_HEIGHT_OFFSET - 4,1,stdin);
 
    // Reconstruct header from read data
    bitmap->header = malloc(bitmap->headerSize);
    memcpy(bitmap->header, data7, BMP_FILE_SIZE_OFFSET);
    memcpy(bitmap->header + BMP_FILE_SIZE_OFFSET, data8, sizeof(int)); 
    memcpy(bitmap->header + BMP_FILE_SIZE_OFFSET + sizeof(int), data1, BMP_HEADER_SIZE_OFFSET-BMP_FILE_SIZE_OFFSET-4);
    memcpy(bitmap->header + BMP_HEADER_SIZE_OFFSET, data2, sizeof(int));
    memcpy(bitmap->header + BMP_HEADER_SIZE_OFFSET + sizeof(int), data3, BMP_WIDTH_OFFSET - BMP_HEADER_SIZE_OFFSET - 4);
    memcpy(bitmap->header + BMP_WIDTH_OFFSET, data4, sizeof(int));
    memcpy(bitmap->header + BMP_HEIGHT_OFFSET, data5, sizeof(int));
    memcpy(bitmap->header + BMP_HEIGHT_OFFSET + sizeof(int), data6, bitmap->headerSize - BMP_HEIGHT_OFFSET - 4);
    return bitmap;
}
 
// Write BMP header to stdout
void write_header(const Bitmap *bmp) {
    fwrite(bmp->header, bmp->headerSize, 1, stdout);
}
 
// Free bitmap memory
void free_bitmap(Bitmap *bmp) {
    free(bmp->header);
    free(bmp);
}
 
// Update bitmap header for scaling operations
void scale(Bitmap *bmp, int scale_factor) {
    bmp->scaleFactor = scale_factor;
    bmp->width = bmp->width * scale_factor;
    bmp->height = bmp->height * scale_factor;
    bmp->fileSize = bmp->headerSize + (bmp->width * bmp->height * 3);
    
    // Update header with new dimensions and file size
    unsigned char *header = malloc(bmp->headerSize);
    memcpy(header, bmp->header, BMP_FILE_SIZE_OFFSET);
    memcpy(header + BMP_FILE_SIZE_OFFSET, &(bmp->fileSize), sizeof(int));
    memcpy(header + BMP_FILE_SIZE_OFFSET + 4, bmp->header + BMP_FILE_SIZE_OFFSET + 4, BMP_WIDTH_OFFSET - BMP_FILE_SIZE_OFFSET - 4);
    memcpy(header + BMP_WIDTH_OFFSET, &(bmp->width), sizeof(int));
    memcpy(header + BMP_HEIGHT_OFFSET, &(bmp->height), sizeof(int));
    memcpy(header + BMP_HEIGHT_OFFSET + 4, bmp->header + BMP_HEIGHT_OFFSET + 4, bmp->headerSize - BMP_HEIGHT_OFFSET - 4);
    memcpy(bmp->header, header, bmp->headerSize);
    free(header);
}
 
// Main filter execution function
void run_filter(void (*filter)(Bitmap *), int scale_factor) {
    Bitmap *bmp = read_header();
 
    if (scale_factor > 1) {
        scale(bmp, scale_factor);
    }
 
    write_header(bmp);
    filter(bmp);
    free_bitmap(bmp);
}
 
 
/******************************************************************************
 * The gaussian blur and edge detection filters.
 *****************************************************************************/
const int gaussian_kernel[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};
 
const int kernel_dx[3][3] = {
    {1, 0, -1},
    {2, 0, -2},
    {1, 0, -1}
};
 
const int kernel_dy[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
};
 
const int gaussian_normalizing_factor = 16;
 
 
Pixel apply_gaussian_kernel(Pixel *row0, Pixel *row1, Pixel *row2) {
    int b = 0, g = 0, r = 0;
    Pixel *rows[3] = {row0, row1, row2};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            b += rows[i][j].blue * gaussian_kernel[i][j];
            g += rows[i][j].green * gaussian_kernel[i][j];
            r += rows[i][j].red * gaussian_kernel[i][j];
        }
    }
 
    b /= gaussian_normalizing_factor;
    g /= gaussian_normalizing_factor;
    r /= gaussian_normalizing_factor;
 
    Pixel new = {
        .blue = b,
        .green = g,
        .red = r
    };
 
    return new;
}
 
 
Pixel apply_edge_detection_kernel(Pixel *row0, Pixel *row1, Pixel *row2) {
    int b_dx = 0, b_dy = 0;
    int g_dx = 0, g_dy = 0;
    int r_dx = 0, r_dy = 0;
    Pixel *rows[3] = {row0, row1, row2};
 
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            b_dx += rows[i][j].blue * kernel_dx[i][j];
            b_dy += rows[i][j].blue * kernel_dy[i][j];
            g_dx += rows[i][j].green * kernel_dx[i][j];
            g_dy += rows[i][j].green * kernel_dy[i][j];
            r_dx += rows[i][j].red * kernel_dx[i][j];
            r_dy += rows[i][j].red * kernel_dy[i][j];
        }
    }
    int b = floor(sqrt(square(b_dx) + square(b_dy)));
    int g = floor(sqrt(square(g_dx) + square(g_dy)));
    int r = floor(sqrt(square(r_dx) + square(r_dy)));
 
    int edge_val = max(r, max(g, b));
    Pixel new = {
        .blue = edge_val,
        .green = edge_val,
        .red = edge_val
    };
 
    return new;
}