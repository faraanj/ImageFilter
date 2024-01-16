#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"


/*
 * Main filter loop.
 */
void gaussian_filter(Bitmap *bmp) {
    // TODO: Complete this function.
    Pixel *buf_original_row1 = malloc(sizeof(Pixel)*bmp->width);
    Pixel *buf_original_row2 = malloc(sizeof(Pixel)*bmp->width);
    Pixel *buf_original_row3 = malloc(sizeof(Pixel)*bmp->width);
    Pixel *buf_transformed = malloc(sizeof(Pixel)*bmp->width);

    // read three rows into original
    fread(buf_original_row1, sizeof(Pixel), bmp->width, stdin);
    fread(buf_original_row2, sizeof(Pixel), bmp->width, stdin);
    fread(buf_original_row3, sizeof(Pixel), bmp->width, stdin);

    for (int j = 1; j < bmp->height - 1; j++) {
        // Apply Gaussian filter for each pixel in the middle row
        for (int i = 1; i < bmp->width - 1; i++) {
            buf_transformed[i] = apply_gaussian_kernel(buf_original_row1 + i - 1, buf_original_row2 + i - 1, buf_original_row3 + i - 1);
        }
        // Handle edge cases for the first and last pixels in the middle row
        buf_transformed[0] = buf_transformed[1];
        buf_transformed[bmp->width -1] = buf_transformed[bmp->width-2];

        // Write transformed row to output
        if (j == 1){
            fwrite(buf_transformed, sizeof(Pixel), bmp->width, stdout);
        }
        fwrite(buf_transformed, sizeof(Pixel), bmp->width, stdout);

        // Shift the buffers up by one row
        Pixel *temp = buf_original_row1;
        buf_original_row1 = buf_original_row2;
        buf_original_row2 = buf_original_row3;
        buf_original_row3 = temp;

        if (j < bmp->height - 2) {
            // Read next row into buf_original_row3
            fread(buf_original_row3, sizeof(Pixel), bmp->width, stdin);
        }
    }

    // Handle the edge cases for the last row
    fwrite(buf_transformed, sizeof(Pixel), bmp->width, stdout);

    // Free allocated memory
    free(buf_original_row1);
    free(buf_original_row2);
    free(buf_original_row3);
    free(buf_transformed);
}

int main() {
    // Run the filter program with gaussian_filter to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(gaussian_filter, 1);
    return 0;
}