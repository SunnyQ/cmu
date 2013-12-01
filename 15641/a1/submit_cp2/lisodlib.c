/******************************************************************************
 * lisodlib.c                                                                  *
 *                                                                             *
 * Description: This file contains the C source code for the library of lisod  *
 *              server. Basically, most request processing and response        *
 *              generating work are done in here.                              *
 *                                                                             *
 * Author: Yang Sun <yksun@cs.cmu.edu>                                         *
 *                                                                             *
 *******************************************************************************/

#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "lisodlib.h"
#include "lisodlog.h"
#include "errno.h"

/* Parse out the value in the format of "target: value" in the request */
char *parse_value(char request[], char target[])
{
    char *key;
    char *value;
    char *line = NULL;

    key = malloc(MAX_HEADER);
    value = malloc(MAX_HEADER);

    line = strtok(request, "\r\n");
    while (line != NULL)
    {
        sscanf(line, "%[^:]: %s", key, value);
        if (key == NULL)
            return NULL;
        else if (strcasecmp(key, target) == 0)
            return value;
        line = strtok(NULL, "\r\n");
    }
    return NULL;
}

/* generate the status line according to the statuscode */
char *generate_statusline(int statuscode)
{
    if (statuscode == 200)
        return "HTTP/1.1 200 OK\r\n";
    else if (statuscode == 404)
        return "HTTP/1.1 404 Not Found\r\n";
    else if (statuscode == 411)
        return "HTTP/1.1 411 Length Required\r\n";
    else if (statuscode == 500)
        return "HTTP/1.1 500 Internal Server Error\r\n";
    else if (statuscode == 501)
        return "HTTP/1.1 501 Not Implemented\r\n";
    else if (statuscode == 503)
        return "HTTP/1.1 503 Service Unavailable\r\n";
    else if (statuscode == 505)
        return "HTTP/1.1 505 HTTP Version not supported\r\n";
    else
        return NULL;
}

/* generate the type according to the file extension. */
char *generate_type(char *filepath)
{
    char *ext;

    ext = strrchr(filepath, '.');
    if (strcasecmp(ext, ".html") == 0 || strcasecmp(ext, ".htm") == 0)
        return "text/html";
    else if (strcasecmp(ext, ".txt") == 0)
        return "text/plain";
    else if (strcasecmp(ext, ".css") == 0)
        return "text/css";
    else if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    else if (strcasecmp(ext, ".png") == 0)
        return "image/png";
    else if (strcasecmp(ext, ".gif") == 0)
        return "image/gif";
    else if (strcasecmp(ext, ".ico") == 0)
        return "image/ico";
    else if (strcasecmp(ext, ".js") == 0)
        return "text/javascript";
    else
        return "text/plain";
}

/* Look up the file in the www folder and process
 * corresponding information */
int process_file(char path[], entity *entity)
{
    char filepath[MAX_HEADER];
    struct stat stat_struct;
    FILE *fp;
    int fd;

    /* If request the root, use index.html instead */
    strncpy(filepath, root, MAX_HEADER);
    if (strcmp(path, "/") == 0)
        strncat(filepath, "/index.html", 11);
    else
    {
        // TODO: process GET tokens, leave for final
        strcat(filepath, path);
    }

    if (!(fp = fopen(filepath, "rb")))
    {
        entity->content_length = 0;
        entity->last_modified = NULL;
        entity->content_type = NULL;
        entity->body = NULL;
        return E_FILENOTFOUND;
    }
    else
    {
        fd = fileno(fp);
        if (fstat(fd, &stat_struct) == -1)
            return E_FILESTAT;

        entity->content_length = stat_struct.st_size;

        entity->last_modified = malloc(200);
        strftime(entity->last_modified, 200, "%a, %d %b %Y %T GMT",
                gmtime((time_t *) &stat_struct.st_mtim));

        entity->content_type = generate_type(filepath);
        entity->body = malloc(entity->content_length);

        if (fread(entity->body, 1, entity->content_length, fp) < 1)
            return E_FILEREAD;
        fclose(fp);
    }

    return 0;
}

/* formalize the response that will be returned to the client */
char *formalize_response(int statuscode, entity *entity, int is_alive)
{
    char *response;

    if (entity == NULL)
        response = malloc(MAX_HEADER);
    else
        response = malloc(MAX_HEADER + entity->content_length);

    strcpy(response, generate_statusline(statuscode));

    if (!is_alive)
        sprintf(response, "%sConnection: close\r\n", response);
    else
        sprintf(response, "%sConnection: keep-alive\r\n", response);

    sprintf(response, "%sDate: %s\r\n", response, current_time());
    sprintf(response, "%sServer: Liso/1.0\r\n", response);

    if (entity != NULL)
    {
        sprintf(response, "%sContent-Length: %d\r\n", response,
                entity->content_length);

        sprintf(response, "%sContent-Type: %s\r\n", response,
                entity->content_type);

        sprintf(response, "%sLast-Modified: %s\r\n", response,
                entity->last_modified);
    }

    strncat(response, "\r\n", 2);

    return response;
}

/* keep sending the data back to the client */
ssize_t send_data(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        // error should write to the log
        if ((nwritten = send(fd, bufp, nleft, 0)) <= 0)
            return -1;
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

/* process the request and dispatch work to each subroutines */
int process_request(int client_sock, char header[], request *req)
{
    char uri[MAX_HEADER];
    char *value;
    char *response;
    int is_alive, rc;
    entity entity;

    memset(req->method, 0, MAX_HEADER);
    memset(uri, 0, MAX_HEADER);
    memset(req->path, 0, MAX_HEADER);
    memset(req->tokens, 0, MAX_HEADER);
    memset(req->version, 0, MAX_HEADER);

    sscanf(header, "%s %s %s", req->method, uri, req->version);
    if (req->method == NULL || uri == NULL || req->version == NULL)
    {
        response = formalize_response(500, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }

    /* log stuff */
    log_fp = fopen(log_file, "a");
    fprintf(log_fp, "%s\t%s\tReceived %s request for %s\n", current_time(),
            DEBUG, req->method, uri);
    fclose(log_fp);

    /* process the connection token */
    value = parse_value(header, "Connection");
    // default to set it to keep-alive
    if (value == NULL)
    {
        write_log(ERROR, "Connection field doesn't exist...");
        response = formalize_response(500, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }
    else if (strncasecmp(value, "Close", 5) == 0)
        is_alive = 0;
    else if (strncasecmp(value, "Keep-Alive", 10) == 0)
        is_alive = 1;
    else
    {
        log_fp = fopen(log_file, "a");
        fprintf(log_fp, "%s\t%s\tRequest for %s, not valid...\n",
                current_time(), ERROR, value);
        fclose(log_fp);

        response = formalize_response(500, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }

    /* version is not correct */
    if (strncasecmp(req->version, "HTTP/1.1", 8))
    {
        response = formalize_response(505, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }

    sscanf(uri, "%[^?]?%s", req->path, req->tokens);

    /* process file path and existence */
    rc = process_file(req->path, &entity);
    if (rc == E_FILENOTFOUND)
    {
        log_fp = fopen(log_file, "a");
        fprintf(log_fp, "%s\t%s\tRequest file %s couldn't be found...\n",
                current_time(), ERROR, req->path);
        fclose(log_fp);

        response = formalize_response(404, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }
    else if (rc == E_FILEREAD || rc == E_FILESTAT)
    {
        log_fp = fopen(log_file, "a");
        fprintf(log_fp, "%s\t%s\tRequest file %s is not available...\n",
                current_time(), ERROR, req->path);
        fclose(log_fp);

        response = formalize_response(500, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }

    /* process GET, HEAD or POST */
    if (strncasecmp(req->method, "GET", 3) == 0)
    {
        response = formalize_response(200, &entity, is_alive);
        send_data(client_sock, response, strlen(response));
        send_data(client_sock, entity.body, entity.content_length);
    }

    else if (strncasecmp(req->method, "HEAD", 4) == 0)
    {
        response = formalize_response(200, &entity, is_alive);
        send_data(client_sock, response, strlen(response));
    }

    else if (strncasecmp(req->method, "POST", 4) == 0)
    {
        // TODO: retrieve POST tokens and run CGI
        response = formalize_response(200, &entity, is_alive);
        send_data(client_sock, response, strlen(response));
        send_data(client_sock, entity.body, entity.content_length);
    }

    else
    {
        response = formalize_response(501, NULL, 0);
        send_data(client_sock, response, strlen(response));
        return -1;
    }

    return 0;
}

