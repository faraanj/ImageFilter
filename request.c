#include "request.h"
#include "response.h"
#include <string.h>

/******************************************************************************
 * ClientState-processing functions
 *****************************************************************************/
ClientState *init_clients(int n)
{
    ClientState *clients = malloc(sizeof(ClientState) * n);
    for (int i = 0; i < n; i++)
    {
        clients[i].sock = -1; // -1 here indicates available entry
        clients[i].num_bytes = 0;
    }
    return clients;
}
/*
 * Remove the client from the client array, free any memory allocated for
 * fields of the ClientState struct, and close the socket.
 */
void remove_client(ClientState *cs)
{
    if (cs->reqData != NULL)
    {
        free(cs->reqData->method);
        free(cs->reqData->path);
        for (int i = 0; i < MAX_QUERY_PARAMS && cs->reqData->params[i].name != NULL; i++)
        {
            free(cs->reqData->params[i].name);
            free(cs->reqData->params[i].value);
        }
        free(cs->reqData);
        cs->reqData = NULL;
    }
    close(cs->sock);
    cs->sock = -1;
    cs->num_bytes = 0;
}

/*
 * Search the first inbuf characters of buf for a network newline ("\r\n").
 * Return the index *immediately after* the location of the '\n'
 * if the network newline is found, or -1 otherwise.
 * Definitely do not use strchr or any other string function in here. (Why not?)
 */
int find_network_newline(const char *buf, int inbuf)
{

    for (int i = 0; i < (inbuf - 1); i++)
    {
        if (buf[i] == '\r')
        {
            if (buf[i + 1] == '\n')
            {
                return i + 2;
            }
        }
    }

    return -1;
}

/*
 * Removes one line (terminated by \r\n) from the client's buffer.
 * Update client->num_bytes accordingly.
 *
 * For example, if `client->buf` contains the string "hello\r\ngoodbye\r\nblah",
 * after calling remove_line on it, buf should contain "goodbye\r\nblah"
 * Remember that the client buffer is *not* null-terminated automatically.
 */
void remove_buffered_line(ClientState *client)
{

    // IMPLEMENT THIS
    int index = 0;
    if ((index = find_network_newline(client->buf, client->num_bytes)) != -1)
    {
        memcpy(client->buf, client->buf + index, client->num_bytes);
        client->num_bytes -= index;
    }
}

/*
 * Read some data into the client buffer. Append new data to data already
 * in the buffer.  Update client->num_bytes accordingly.
 * Return the number of bytes read in, or -1 if the read failed.

 * Be very careful with memory here: there might be existing data in the buffer
 * that you don't want to overwrite, and you also don't want to go past
 * the end of the buffer, and you should ensure the string is null-terminated.
 */
int read_from_client(ClientState *client)
{
    // IMPLEMENT THIS
    int new_data_size = MAXLINE - client->num_bytes - 1;
    int num_read = 0;
    if (new_data_size != 0)
    {
        num_read = read(client->sock, client->buf + client->num_bytes, new_data_size);
    }
    if (num_read == 0)
    {
        return -1;
    }
    client->num_bytes += num_read;
    client->buf[client->num_bytes] = '\0';
    return num_read;
}

/*****************************************************************************
 * Parsing the start line of an HTTP request.
 ****************************************************************************/
// Helper function declarations.
void parse_query(ReqData *req, const char *str);
void update_fdata(Fdata *f, const char *str);
void fdata_free(Fdata *f);
void log_request(const ReqData *req);

/* If there is a full line (terminated by a network newline (CRLF))
 * then use this line to initialize client->reqData
 * Return 0 if a full line has not been read, 1 otherwise.
 */
int parse_req_start_line(ClientState *client)
{

    // IMPLEMENT THIS
    int index = 0;
    if ((index = find_network_newline(client->buf, client->num_bytes)) != -1)
    {
        client->buf[index - 2] = '\0';
        client->reqData = (ReqData *)malloc(sizeof(ReqData));
        parse_query(client->reqData, client->buf);
        log_request(client->reqData);
        return 1;
    }
    else
    {
        if (read_from_client(client) == -1)
        {
            return -1;
        }
        return 0;
    }
    // This part is just for debugging purposes.
    return 1;
}

/*
 * Initializes req->params from the key-value pairs contained in the given
 * string.
 * Assumes that the string is the part after the '?' in the HTTP request target,
 * e.g., name1=value1&name2=value2.
 */
void parse_query(ReqData *req, const char *str)
{
    // IMPLEMENT THIS

    for (int i = 0; i < MAX_QUERY_PARAMS; i++)
    {
        req->params[i].name = NULL;
        req->params[i].value = NULL;
    }

    int index = 0;
    int length = strlen(str);
    int i = 0;

    // get method
    if (str[i] == 'G')
    {
        req->method = (char *)malloc(sizeof(char) * 4);
        char *get = "GET\0";
        strncpy(req->method, get, 4);
        i += 4;
    }
    if (str[i] == 'P')
    {
        req->method = (char *)malloc(sizeof(char) * 5);
        char *post = "POST\0";
        strncpy(req->method, post, 5);
        i += 5;
    }

    // get target
    int k = 0;
    if (str[i] == '/')
    {
        const char *target_str = str + i;
        while (str[i] != ' ' && str[i] != '?' && str[i] != '\0')
        {
            i++;
            k++;
        }
        req->path = (char *)malloc(k + 1);
        strncpy(req->path, target_str, k);
        req->path[k] = '\0';
    }

    // get name_value pair
    if (strcmp(req->method, GET) == 0)
    {
        while (i < length && index < MAX_QUERY_PARAMS)
        {
            while (str[i] != '?' && str[i] != '\0' && str[i] != '&' && str[i] != ' ')
            {
                i++;
            }

            if (str[i] == '&' || str[i] == '?')
            {
                update_fdata(&req->params[index], str + i + 1);
                index++;
                i++;
            }
            else
            {
                break;
            }
        }
    }
}

void update_fdata(Fdata *f, const char *str)
{
    // get name
    int i = 0;
    while (str[i] != '=' && str[i] != ' ')
    {
        i++;
    }
    f->name = (char *)malloc(i + 1);
    strncpy(f->name, str, i);
    f->name[i] = '\0';

    // get value
    int j = 0;
    str = str + i + 1;
    while (str[j] != '&' && str[j] != ' ')
    {
        j++;
    }
    f->value = (char *)malloc(j + 1);
    strncpy(f->value, str, j);
    f->value[j] = '\0';
}

/*
 * Print information stored in the given request data to stderr.
 */
void log_request(const ReqData *req)
{
    fprintf(stderr, "Request parsed: [%s] [%s]\n", req->method, req->path);
    for (int i = 0; i < MAX_QUERY_PARAMS && req->params[i].name != NULL; i++)
    {
        fprintf(stderr, "  %s -> %s\n",
                req->params[i].name, req->params[i].value);
    }
}

/******************************************************************************
 * Parsing multipart form data (image-upload)
 *****************************************************************************/

char *get_boundary(ClientState *client)
{
    int len_header = strlen(POST_BOUNDARY_HEADER);

    while (1)
    {
        int where = find_network_newline(client->buf, client->num_bytes);
        if (where > 0)
        {
            if (where < len_header || strncmp(POST_BOUNDARY_HEADER, client->buf, len_header) != 0)
            {
                remove_buffered_line(client);
            }
            else
            {
                // We've found the boundary string!
                // We are going to add "--" to the beginning to make it easier
                // to match the boundary line later
                char *boundary = malloc(where - len_header + 1);
                strncpy(boundary, "--", where - len_header + 1);
                strncat(boundary, client->buf + len_header, where - len_header - 1);
                boundary[where - len_header] = '\0';
                return boundary;
            }
        }
        else
        {
            // Need to read more bytes
            if (read_from_client(client) <= 0)
            {
                // Couldn't read; this is a bad request, so give up.
                return NULL;
            }
        }
    }
    return NULL;
}

char *get_bitmap_filename(ClientState *client, const char *boundary)
{
    int len_boundary = strlen(boundary);

    // Read until finding the boundary string.
    while (1)
    {
        int where = find_network_newline(client->buf, client->num_bytes);
        if (where > 0)
        {
            if (where < len_boundary + 2 ||
                strncmp(boundary, client->buf, len_boundary) != 0)
            {
                remove_buffered_line(client);
            }
            else
            {
                // We've found the line with the boundary!
                remove_buffered_line(client);
                break;
            }
        }
        else
        {
            // Need to read more bytes
            if (read_from_client(client) <= 0)
            {
                // Couldn't read; this is a bad request, so give up.
                return NULL;
            }
        }
    }

    int where = find_network_newline(client->buf, client->num_bytes);

    client->buf[where - 1] = '\0'; // Used for strrchr to work on just the single line.
    char *raw_filename = strrchr(client->buf, '=') + 2;
    int len_filename = client->buf + where - 3 - raw_filename;
    char *filename = malloc(len_filename + 1);
    strncpy(filename, raw_filename, len_filename);
    filename[len_filename] = '\0';

    // Restore client->buf
    client->buf[where - 1] = '\n';
    remove_buffered_line(client);
    return filename;
}

/*
 * Read the file data from the socket and write it to the file descriptor
 * file_fd.
 * You know when you have reached the end of the file in one of two ways:
 *    - search for the boundary string in each chunk of data read
 * (Remember the "\r\n" that comes before the boundary string, and the
 * "--\r\n" that comes after.)
 *    - extract the file size from the bitmap data, and use that to determine
 * how many bytes to read from the socket and write to the file
 */
int save_file_upload(ClientState *client, const char *boundary, int file_fd)
{
    // Read in the next two lines: Content-Type line, and empty line
    remove_buffered_line(client);
    remove_buffered_line(client);

    if (write(file_fd, client->buf, client->num_bytes) != client->num_bytes)
    {
        perror("write");
        return -1;
    }

    char buffer[MAXLINE];
    int total_bytes_read = client->num_bytes;
    memmove(client->buf, client->buf + client->num_bytes, client->num_bytes - client->num_bytes);
    client->num_bytes -= client->num_bytes;
    while (1)
    {
        int bytes_read = read_from_client(client);
        if (bytes_read < 0)
        {
            return -1;
        }

        memcpy(buffer, client->buf, bytes_read);

        char *boundary_pos = strstr(buffer, boundary);
        if (boundary_pos != NULL)
        {
            int bytes_to_write = boundary_pos - buffer - 2;
            if (write(file_fd, buffer, bytes_to_write) != bytes_to_write)
            {
                perror("write");
                return -1;
            }
            total_bytes_read += bytes_to_write;
            break;
        }
        else
        {
            if (write(file_fd, buffer, bytes_read) != bytes_read)
            {
                perror("write");
                return -1;
            }
            total_bytes_read += bytes_read;
        }
        memmove(client->buf, client->buf + bytes_read, client->num_bytes - bytes_read);
        client->num_bytes -= bytes_read;
    }

    return total_bytes_read;
}