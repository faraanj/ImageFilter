#ifndef REQUEST_H_
#define REQUEST_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_QUERY_PARAMS 5
#define MAXLINE 1024

// String constants for parsing HTTP requests.
#define GET "GET"
#define POST "POST"
#define MAIN_HTML "/main.html"
#define IMAGE_FILTER "/image-filter"
#define IMAGE_UPLOAD "/image-upload"

#define IMAGE_DIR "images/"
#define FILTER_DIR "filters/"

#define POST_BOUNDARY_HEADER "Content-Type: multipart/form-data; boundary="

// A struct representing a key-value pair as a query params
typedef struct formdata
{
    char *name;
    char *value;
} Fdata;

/* A struct storing three parts of the first line of an HTTP request.
 *
 * The params array should be parsed from the 'query' field.
 * If there are fewer than MAX_QUERY_PARAMS, each remaining Fdata
 * value should have its fields set to NULL.
 */

typedef struct
{
    char *method;                   // Either "GET" or "POST"
    char *path;                     // Request path, e.g. "main.html" or "image-filter"
    Fdata params[MAX_QUERY_PARAMS]; // An array of query params.
} ReqData;

typedef struct
{
    int sock;          // The socket fd used to communicate with the client.
    char buf[MAXLINE]; // A buffer of the data read from the client request,
                       // PLUS space for a null-terminator
                       // (so at most MAXLINE - 1 bytes from client).
    int num_bytes;     // The number of bytes currently in the buffer
                       // (must be between 0 and MAXLINE - 1).
    ReqData *reqData;  // The data parsed from the first line of the HTTP
                       // request from the client.
} ClientState;

/*
 * Returns an array of ClientStates of the given size.
 */
ClientState *init_clients(int n);

/*
 * Frees memory allocated for the given client fields.
 */
void remove_client(ClientState *cs);

/******************************************************************************
 * Functions for directly maniputing client buffers.
 *****************************************************************************/

/*
 * Read some data into the client buffer. Update client->num_bytes accordingly.
 * Return the number of bytes read in, or -1 if the read failed.
 */
int read_from_client(ClientState *client);

/******************************************************************************
 * Functions for parsing parts of the HTTP request
 *****************************************************************************/

/*
 * Parse the start line of an HTTP request, storing the data in client->reqData.
 */
int parse_req_start_line(ClientState *client);

/*
 * Return the boundary string for this request.
 * This should be returned in a separate dynamically-allocated,
 * null-terminated string 
 *
 * Returns NULL if no boundary string is found.
 */
char *get_boundary(ClientState *client);

/*
 * Return the filename of the bitmap image for this request, using the
 * given boundary string to detect sections. 
 * Returns NULL if no filename string is found.
 */
char *get_bitmap_filename(ClientState *client, const char *boundary);

/*
 * Read the bitmap image data from the request and write it
 * to the given fd (representing a file).
 *
 * Use the boundary string to determine when the file data stops
 *
 * Return 0 if boundary string was found at the end of the request,
 * and -1 otherwise (this indicates a bad request).
 *
 */
int save_file_upload(ClientState *client, const char *boundary, int file_fd);

#endif /* REQUEST_H_*/
