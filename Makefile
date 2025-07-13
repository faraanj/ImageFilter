# You should change the value of PORT
PORT = 30001
CC = gcc
CFLAGS =  -DPORT=${PORT} -g -Wall -std=gnu99 


# Note that this Makefile populates the images/ and filters/ directories
# for the server.
all: image_server images filters

image_server: image_server.o response.o request.o socket.o
	${CC} ${CFLAGS} -o $@ $^

.c.o: response.h request.h socket.h
	${CC} ${CFLAGS}  -c $<

# Create images directory and copy sample image
images:
	mkdir -p images
	cp sam.bmp images

# Build filter executables
filters: filters/copy filters/greyscale filters/gaussian_blur filters/edge_detection

filters/copy: filters/copy.c filters/bitmap.c filters/bitmap.h
	${CC} ${CFLAGS} -o $@ filters/copy.c filters/bitmap.c -lm

filters/greyscale: filters/greyscale.c filters/bitmap.c filters/bitmap.h
	${CC} ${CFLAGS} -o $@ filters/greyscale.c filters/bitmap.c -lm

filters/gaussian_blur: filters/gaussian_blur.c filters/bitmap.c filters/bitmap.h
	${CC} ${CFLAGS} -o $@ filters/gaussian_blur.c filters/bitmap.c -lm

filters/edge_detection: filters/edge_detection.c filters/bitmap.c filters/bitmap.h
	${CC} ${CFLAGS} -o $@ filters/edge_detection.c filters/bitmap.c -lm

clean:
	rm -f *.o image_server
	rm -f filters/copy filters/greyscale filters/gaussian_blur filters/edge_detection
