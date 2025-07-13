#define MAXLINE 1024
#define IMAGE_DIR "images/"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include "response.h"
#include "request.h"

// Functions for internal use only
void write_image_list(int fd);
void write_image_response_header(int fd);

// Write the main.html response to the given fd
void main_html_response(int fd)
{
    char *header =
        "HTTP/1.1 200 OK\r\n"
        "Content-type: text/html\r\n\r\n";

    if (write(fd, header, strlen(header)) == -1)
    {
        perror("write");
    }

    FILE *in_fp = fopen("main.html", "r");
    char buf[MAXLINE];
    while (fgets(buf, MAXLINE, in_fp) > 0)
    {
        if (write(fd, buf, strlen(buf)) == -1)
        {
            perror("write");
        }
        // Insert dynamic Javascript into the HTML page
        // This assumes there's only one "<script>" element in the page.
        if (strncmp(buf, "<script>", strlen("<script>")) == 0)
        {
            write_image_list(fd);
        }
    }
    fclose(in_fp);
}

/*
 * Write image directory contents to the given fd, in the format
 * "var filenames = ['<filename1>', '<filename2>', ...];\n"
 *
 * This is actually a line of Javascript that's used to populate the form
 * when the webpage is loaded.
 */
void write_image_list(int fd)
{
    DIR *d = opendir(IMAGE_DIR);
    struct dirent *dir;

    dprintf(fd, "var filenames = [");
    if (d != NULL)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                dprintf(fd, "'%s', ", dir->d_name);
            }
        }
        closedir(d);
    }
    dprintf(fd, "];\n");
}

// Process image filter requests
void image_filter_response(int fd, const ReqData *reqData)
{
    char *filter_name = NULL;
    char *image_name = NULL;
    char *filter_value = NULL;
    char *image_value = NULL;
    
    // Find filter and image parameters regardless of order
    for (int i = 0; i < MAX_QUERY_PARAMS && reqData->params[i].name != NULL; i++) {
        if (strcmp(reqData->params[i].name, "filter") == 0) {
            filter_name = reqData->params[i].name;
            filter_value = reqData->params[i].value;
        } else if (strcmp(reqData->params[i].name, "image") == 0) {
            image_name = reqData->params[i].name;
            image_value = reqData->params[i].value;
        }
    }
    
    // Check if both parameters were found
    if (filter_name == NULL || image_name == NULL || filter_value == NULL || image_value == NULL) {
        bad_request_response(fd, "Filter and image parameters are required");
        return;
    }

    char *image_file = malloc(strlen(IMAGE_DIR) + strlen(image_value) + 1);
    strcpy(image_file, IMAGE_DIR);
    strcat(image_file, image_value);
    char *filter_file = malloc(strlen(FILTER_DIR) + strlen(filter_value) + 1);
    strcpy(filter_file, FILTER_DIR);
    strcat(filter_file, filter_value);

    // Check for path traversal attacks
    if (strchr(filter_value, '/') != NULL || strchr(image_value, '/') != NULL)
    {
        bad_request_response(fd, "Filter and image parameters cannot contain a slash character");
        free(image_file);
        free(filter_file);
        return;
    }
    
    // Check if files exist and have correct permissions
    if (access(filter_file, X_OK) == -1 || access(image_file, R_OK) == -1)
    {
        bad_request_response(fd, "Filter value or image does not exist");
        free(image_file);
        free(filter_file);
        return;
    }

    // Execute the filter
    FILE *f = fopen(image_file, "r");
    if (f == NULL) {
        bad_request_response(fd, "Could not open image file");
        free(image_file);
        free(filter_file);
        return;
    }

    write_image_response_header(fd);
    dup2(fileno(f), STDIN_FILENO);
    fclose(f);
    dup2(fd, STDOUT_FILENO);

    execl(filter_file, filter_value, NULL);
    perror("execl");
    exit(1);
}

// Respond to an image-upload request
void image_upload_response(ClientState *client)
{
    // First, extract the boundary string for the request
    char *boundary = get_boundary(client);
    if (boundary == NULL)
    {
        bad_request_response(client->sock, "Couldn't find boundary string in request.");
        exit(1);
    }
    fprintf(stderr, "Boundary string: %s\n", boundary);

    // Use the boundary string to extract the name of the uploaded bitmap file
    char *filename = get_bitmap_filename(client, boundary);
    if (filename == NULL)
    {
        bad_request_response(client->sock, "Couldn't find bitmap filename in request.");
        close(client->sock);
        exit(1);
    }

    // If the file already exists, send a Bad Request error to the user
    char *path = malloc(strlen(IMAGE_DIR) + strlen(filename) + 1);
    strcpy(path, IMAGE_DIR);
    strcat(path, filename);

    fprintf(stderr, "Bitmap path: %s\n", path);

    if (access(path, F_OK) >= 0)
    {
        bad_request_response(client->sock, "File already exists.");
        exit(1);
    }

    FILE *file = fopen(path, "wb");
    save_file_upload(client, boundary, fileno(file));
    fclose(file);
    free(boundary);
    free(filename);
    free(path);
    see_other_response(client->sock, MAIN_HTML);
}

// Write the header for a bitmap image response to the given fd
void write_image_response_header(int fd)
{
    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/bmp\r\n"
        "Content-Disposition: attachment; filename=\"output.bmp\"\r\n\r\n";

    write(fd, response, strlen(response));
}

void not_found_response(int fd)
{
    char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Page not found.\r\n";
    write(fd, response, strlen(response));
}

void internal_server_error_response(int fd, const char *message)
{
    char *response =
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
        "<html><head>\r\n"
        "<title>500 Internal Server Error</title>\r\n"
        "</head><body>\r\n"
        "<h1>Internal Server Error</h1>\r\n"
        "<p>%s<p>\r\n"
        "</body></html>\r\n";

    dprintf(fd, response, message);
}

void bad_request_response(int fd, const char *message)
{
    char *response_header =
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n\r\n";
    char *response_body =
        "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
        "<html><head>\r\n"
        "<title>400 Bad Request</title>\r\n"
        "</head><body>\r\n"
        "<h1>Bad Request</h1>\r\n"
        "<p>%s<p>\r\n"
        "</body></html>\r\n";
    char header_buf[MAXLINE];
    char body_buf[MAXLINE];
    sprintf(body_buf, response_body, message);
    sprintf(header_buf, response_header, strlen(body_buf));
    write(fd, header_buf, strlen(header_buf));
    write(fd, body_buf, strlen(body_buf));
    // Hack: sleep to avoid connection reset message
    sleep(1);
}

void see_other_response(int fd, const char *other)
{
    char *response =
        "HTTP/1.1 303 See Other\r\n"
        "Location: %s\r\n\r\n";

    dprintf(fd, response, other);
}