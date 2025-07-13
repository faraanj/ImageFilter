#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

// Main filter loop - reads pixels one at a time and writes them out
void copy_filter(Bitmap *bmp) {
    Pixel *p = malloc(sizeof(Pixel));

    for (int i = 0; i < bmp->height; i++){
        for (int j = 0; j < bmp->width; j++){
            fread(p, sizeof(Pixel), 1, stdin);
            fwrite(p,sizeof(Pixel),1,stdout);
        }
    }
    free(p);
}

int main() {
    run_filter(copy_filter, 1);
    return 0;
}