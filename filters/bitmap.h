#ifndef BITMAP_H_
#define BITMAP_H_
 
// Use the following offsets to index into the `header`
// field of the Bitmap struct.
#define BMP_FILE_SIZE_OFFSET 2
#define BMP_HEADER_SIZE_OFFSET 10
#define BMP_WIDTH_OFFSET 18
#define BMP_HEIGHT_OFFSET 22
 
typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;
 
typedef struct {
    int scaleFactor;          // The scale factor
    int fileSize;            // The size of the file
    int headerSize;          // The size of the header.
    unsigned char *header;   // The contents of the image header.
    int width;               // The width of the image, in pixels.
    int height;              // The height of the image, in pixels.
} Bitmap;
 
 
/*
 * The "main" function.
 *
 * Run a given filter function, and apply a scale factor if necessary.
 * You don't need to modify this function to make it work with any of
 * the filters for this assignment.
 */
void run_filter(void (*filter)(Bitmap *), int scale_factor);
void greyscale(Bitmap *bmp);
void gaussian_filter(Bitmap *bmp);
void edge_detection(Bitmap *bmp);
void scaling(Bitmap *bmp);
 
// Macros and functions for performing the two multi-row filters.
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define square(a) ((a) * (a))
 
 
/*
 * Functions for the row-buffered filters ("convolutions")
 * -------------------------------------------------------
 */
Pixel apply_gaussian_kernel(Pixel *row0, Pixel *row1, Pixel *row2);
Pixel apply_edge_detection_kernel(Pixel *row0, Pixel *row1, Pixel *row2);
 
#endif /* BITMAP_H_*/