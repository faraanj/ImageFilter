#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"


/*
 * Main filter loop.
 */
void greyscale(Bitmap *bmp) {
    // TODO: Complete this function.
    Pixel *p = malloc(sizeof(Pixel));

    for (int i = 0; i < bmp->height; i++){
        for (int j = 0; j < bmp->width; j++){
            fread(p, sizeof(Pixel), 1, stdin);
            unsigned char average = (p->blue + p->green + p->red) / 3;
            p->blue = average;
            p->green = average;
            p->red = average;
            fwrite(p,sizeof(Pixel),1,stdout);
        }
    }
    free(p);

}
int main() {
    // Run the filter program with copy_filter to process the pixels.
    // You shouldn't need to change this implementation.
    run_filter(greyscale, 1);
    return 0;
}