#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

// Main filter loop - converts pixels to greyscale
void greyscale(Bitmap *bmp) {
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
    run_filter(greyscale, 1);
    return 0;
}