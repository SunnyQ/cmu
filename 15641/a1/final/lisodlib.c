/*
 * This serves as the lisod backend library. Most of
 * the backend work are done in this file. The entry
 * parameters this file concerns are the header that
 * parsed out from the request and the select pool.
 *
 *      Author: Yang Sun (yksun@cs.cmu.edu)
 */

#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include "lisodlib.h"
#include "lisodlog.h"
#include "errno.h"

/* Parse out the value in the format of "target: value" in the request */
char *parse_value(char request[], char target[])
{
    char *key;
    char *value;
    char *line = NULL;
    char tmp[MAX_HEADER];

    key = malloc(MAX_HEADER);
    value = malloc(MAX_HEADER);

    /* in case that request will be modified */
    strncpy(tmp, request, MAX_HEADER);

    line = strtok(tmp, "\r\n");
    while (line != NULL)
    {
        sscanf(line, "%[^:]: %[^\t\n]", key, value);
        if (key == NULL)
        {
            memset(tmp, 0, MAX_HEADER);
            return NULL;
        }
        else if (strcasecmp(key, target) == 0)
        {
            memset(tmp, 0, MAX_HEADER);
            return value;
        }
        line = strtok(NULL, "\r\n");
    }
    memset(tmp, 0, MAX_HEADER);
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
    else if (statuscode == 413)
        return "HTTP/1.1 413 Request Entity Too Large\r\n";
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
        strcat(filepath, path);

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
ssize_t send_data(SSL *ssl_context, int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        nwritten =
                (ssl_context == NULL) ?
                        write(fd, bufp, nleft) :
                        SSL_write(ssl_context, bufp, nleft);
        if (nwritten > 0)
        {
            nleft -= nwritten;
            bufp += nwritten;
        }
    }
    return n;
}

/* Look up the fd in the pool to determine if it is SSL channel */
SSL *ssl_lookup(pool *p, int fd)
{
    int i;
    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (p->clientfd[i] == fd && p->httpscontext[i] != NULL)
            return p->httpscontext[i];
    }
    return NULL;
}

/* Reset the memory content for the request */
void clear_mem(request *req)
{
    memset(req->method, 0, BUF_SIZE);
    memset(req->uri, 0, BUF_SIZE);
    memset(req->tokens, 0, BUF_SIZE);
    memset(req->version, 0, BUF_SIZE);
    memset(req->connection, 0, BUF_SIZE);
    memset(req->content_type, 0, BUF_SIZE);
    memset(req->path_info, 0, BUF_SIZE);
    memset(req->query_str, 0, BUF_SIZE);
    memset(req->remote_addr, 0, BUF_SIZE);
    memset(req->http_accept, 0, BUF_SIZE);
    memset(req->http_referer, 0, BUF_SIZE);
    memset(req->http_accept_encoding, 0, BUF_SIZE);
    memset(req->http_accept_language, 0, BUF_SIZE);
    memset(req->http_accept_charset, 0, BUF_SIZE);
    memset(req->http_host, 0, BUF_SIZE);
    memset(req->http_cookie, 0, BUF_SIZE);
    memset(req->http_user_agent, 0, BUF_SIZE);
    memset(req->server_port, 0, BUF_SIZE);
}

/* process the request and dispatch work to each subroutines */
int process_request(pool *p, int client_sock, char header[], request *req)
{
    char *value;
    char *response;
    int is_alive, rc;
    entity entity;
    SSL *ssl_context;

    clear_mem(req);
    ssl_context = ssl_lookup(p, client_sock);

    sscanf(header, "%s %s %s", req->method, req->uri, req->version);
    if (req->method == NULL || req->uri == NULL || req->version == NULL)
    {
        response = formalize_response(500, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }

    /* log stuff */
    sprintf(log_msg, "Received %s request for %s", req->method, req->uri);
    write_log(DEBUG);

    /* process the connection token */
    value = parse_value(header, "Connection");

    /* if connection field doesn't exist, return 500 error */
    if (value == NULL)
    {
        sprintf(log_msg, "Connection field doesn't exist...");
        write_log(ERROR);

        response = formalize_response(500, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }
    else if (strncasecmp(value, "Close", 5) == 0)
        is_alive = 0;
    else if (strncasecmp(value, "Keep-Alive", 10) == 0)
        is_alive = 1;
    else
    {
        sprintf(log_msg, "Request for %s, not valid...", value);
        write_log(ERROR);

        response = formalize_response(500, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }

    /* version is not correct */
    if (strncasecmp(req->version, "HTTP/1.1", 8))
    {
        response = formalize_response(505, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }

    /* determine cgi request */
    if (strncasecmp(req->uri, "/cgi/", 4) == 0)
        return run_cgi(p, client_sock, header, req);

    /* process file path and existence */
    rc = process_file(req->uri, &entity);
    if (rc == E_FILENOTFOUND)
    {
        sprintf(log_msg, "Request file %s couldn't be found...", req->uri);
        write_log(ERROR);
        response = formalize_response(404, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }
    else if (rc == E_FILEREAD || rc == E_FILESTAT)
    {
        sprintf(log_msg, "Request file %s is not available...", req->uri);
        write_log(ERROR);

        response = formalize_response(500, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }

    /* process GET, HEAD or POST */
    if (strncasecmp(req->method, "GET", 3) == 0)
    {
        response = formalize_response(200, &entity, is_alive);
        send_data(ssl_context, client_sock, response, strlen(response));
        send_data(ssl_context, client_sock, entity.body, entity.content_length);
    }

    else if (strncasecmp(req->method, "HEAD", 4) == 0)
    {
        response = formalize_response(200, &entity, is_alive);
        send_data(ssl_context, client_sock, response, strlen(response));
    }

    else if (strncasecmp(req->method, "POST", 4) == 0)
    {
        response = formalize_response(200, &entity, is_alive);
        send_data(ssl_context, client_sock, response, strlen(response));
        send_data(ssl_context, client_sock, entity.body, entity.content_length);
    }

    else
    {
        response = formalize_response(501, NULL, 0);
        send_data(ssl_context, client_sock, response, strlen(response));
        return -1;
    }

    return 0;
}

/* error handler for execv */
void execve_error_handler()
{
    switch (errno)
    {
    case E2BIG:
        sprintf(log_msg, "The total number of bytes in the environment "
                "(envp) and argument list (argv) is too large.");
        write_log(ERROR);
        return;
    case EACCES:
        sprintf(log_msg, "Execute permission is denied for the file or a "
                "script or ELF interpreter.");
        write_log(ERROR);
        return;
    case EFAULT:
        sprintf(log_msg,
                "filename points outside your accessible address space.");
        write_log(ERROR);
        return;
    case EINVAL:
        sprintf(log_msg,
                "An ELF executable had more than one PT_INTERP segment "
                        "(i.e., tried to name more than one interpreter).");
        write_log(ERROR);
        return;
    case EIO:
        sprintf(log_msg, "An I/O error occurred.");
        write_log(ERROR);
        return;
    case EISDIR:
        sprintf(log_msg, "An ELF interpreter was a directory.");
        write_log(ERROR);
        return;
    case ELIBBAD:
        sprintf(log_msg, "An ELF interpreter was not in a recognised format.");
        write_log(ERROR);
        return;
    case ELOOP:
        sprintf(log_msg, "Too many symbolic links were encountered in "
                "resolving filename or the name of a script "
                "or ELF interpreter.");
        write_log(ERROR);
        return;
    case EMFILE:
        sprintf(log_msg, "The process has the maximum number of files open");
        write_log(ERROR);
        return;
    case ENAMETOOLONG:
        sprintf(log_msg, "filename is too long.");
        write_log(ERROR);
        return;
    case ENFILE:
        sprintf(log_msg, "The system limit on the total number of open "
                "files has been reached.");
        write_log(ERROR);
        return;
    case ENOENT:
        sprintf(log_msg, "The file filename or a script or ELF interpreter "
                "does not exist, or a shared library needed for "
                "file or interpreter cannot be found.");
        write_log(ERROR);
        return;
    case ENOEXEC:
        sprintf(log_msg, "An executable is not in a recognised format, is "
                "for the wrong architecture, or has some other "
                "format error that means it cannot be "
                "executed.");
        write_log(ERROR);
        return;
    case ENOMEM:
        sprintf(log_msg, "Insufficient kernel memory was available.");
        write_log(ERROR);
        return;
    case ENOTDIR:
        sprintf(log_msg, "A component of the path prefix of filename or a "
                "script or ELF interpreter is not a directory.");
        write_log(ERROR);
        return;
    case EPERM:
        sprintf(log_msg, "The file system is mounted nosuid, the user is "
                "not the superuser, and the file has an SUID or "
                "SGID bit set.");
        write_log(ERROR);
        return;
    case ETXTBSY:
        sprintf(log_msg,
                "Executable was open for writing by one or more processes.");
        write_log(ERROR);
        return;
    default:
        sprintf(log_msg, "Unkown error occurred with execve().");
        write_log(ERROR);
        return;
    }
}

/* This is the function to process cgi request */
int run_cgi(pool *p, int client_sock, char header[], request *req)
{
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    int i;
    SSL *ssl_context;
    struct sockaddr_in *addr;

    /*************** Organize ENVP array **************/

    char content_length[BUF_SIZE];
    if (req->content_length == 0)
        strcpy(content_length, "CONTENT_LENGTH=");
    else
        sprintf(content_length, "CONTENT_LENGTH=%d", req->content_length);

    sprintf(req->content_type, "CONTENT_TYPE=%s",
            parse_value(header, "Content-Type") == NULL ?
                    "" : parse_value(header, "Content-Type"));

    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (p->clientfd[i] == client_sock)
        {
            addr = p->cliaddr[i];
            sprintf(req->remote_addr, "REMOTE_ADDR=%s",
                    inet_ntoa(addr->sin_addr));
            ssl_context = p->httpscontext[i];
        }
    }

    char temp_method[BUF_SIZE];
    strcpy(temp_method, req->method);
    sprintf(req->method, "REQUEST_METHOD=%s", temp_method);

    char temp_query[BUF_SIZE] = "";
    char temp_pathinfo[BUF_SIZE] = "";
    sscanf(req->uri, "/cgi%[^?]?%s", temp_pathinfo, temp_query);
    sprintf(req->path_info, "PATH_INFO=%s", temp_pathinfo);
    sprintf(req->query_str, "QUERY_STRING=%s", temp_query);

    strcpy(req->uri, "REQUEST_URI=/cgi");
    strncat(req->uri, temp_pathinfo, BUF_SIZE);

    sprintf(req->server_port, "SERVER_PORT=%d",
            ssl_context != NULL ? https_port : http_port);

    char https_indicator[BUF_SIZE];
    sprintf(https_indicator, "HTTPS=%s", ssl_context != NULL ? "on" : "off");

    sprintf(req->http_accept, "HTTP_ACCEPT=%s",
            parse_value(header, "Accept") == NULL ?
                    "" : parse_value(header, "Accept"));

    sprintf(req->http_referer, "HTTP_REFERER=%s",
            parse_value(header, "Referer") == NULL ?
                    "" : parse_value(header, "Referer"));

    sprintf(req->http_accept_encoding, "HTTP_ACCEPT_ENCODING=%s",
            parse_value(header, "Accept-Encoding") == NULL ?
                    "" : parse_value(header, "Accept-Encoding"));

    sprintf(req->http_accept_language, "HTTP_ACCEPT_LANGUAGE=%s",
            parse_value(header, "Accept-Language") == NULL ?
                    "" : parse_value(header, "Accept-Language"));

    sprintf(req->http_accept_charset, "HTTP_ACCEPT_CHARSET=%s",
            parse_value(header, "Accept-Charset") == NULL ?
                    "" : parse_value(header, "Accept-Charset"));

    sprintf(req->http_host, "HTTP_HOST=%s",
            parse_value(header, "Host") == NULL ?
                    "" : parse_value(header, "Host"));

    sprintf(req->http_cookie, "HTTP_COOKIE=%s",
            parse_value(header, "Cookie") == NULL ?
                    "" : parse_value(header, "Cookie"));

    sprintf(req->http_user_agent, "HTTP_USER_AGENT=%s",
            parse_value(header, "User-Agent") == NULL ?
                    "" : parse_value(header, "User-Agent"));

    sprintf(req->connection, "HTTP_CONNECTION=%s",
            parse_value(header, "Connection") == NULL ?
                    "" : parse_value(header, "Connection"));

    char *envp[] =
    { content_length, req->content_type, "GATEWAY_INTERFACE=CGI/1.1",
            req->query_str, req->path_info, req->remote_addr, req->method,
            req->uri, "SCRIPT_NAME=/cgi", req->server_port, https_indicator,
            "SERVER_PROTOCOL=HTTP/1.1", "SERVER_SOFTWARE=Liso/1.0",
            req->http_accept, req->http_referer, req->http_accept_encoding,
            req->http_accept_language, req->http_accept_charset, req->http_host,
            req->http_cookie, req->http_user_agent, req->connection, NULL };

    /*************** ENVP array is done **************/

    /* 0 can be read from, 1 can be written to */
    if (pipe(stdin_pipe) < 0)
    {
        sprintf(log_msg, "Error piping for stdin.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    if (pipe(stdout_pipe) < 0)
    {
        sprintf(log_msg, "Error piping for stdout.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    /*************** BEGIN FORK **************/
    pid = fork();
    /* not good */
    if (pid < 0)
    {
        sprintf(log_msg, "Something really bad happened when fork()ing.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    /* child, setup environment, execve */
    if (pid == 0)
    {
        /*************** BEGIN EXECVE ****************/
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        dup2(stdout_pipe[1], fileno(stdout));
        dup2(stdin_pipe[0], fileno(stdin));

        /* pretty much no matter what, if it returns bad things happened... */
        char *argv[] =
        { cgi_root, NULL };
        if (execve(cgi_root, argv, envp))
        {
            execve_error_handler();
            sprintf(log_msg, "Error executing execve syscall.");
            write_log(ERROR);
            return EXIT_FAILURE;
        }
        /*************** END EXECVE ****************/
    }

    if (pid > 0)
    {
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);

        if (write(stdin_pipe[1], req->body, strlen(req->body)) < 0)
        {
            sprintf(log_msg, "Error writing to spawned CGI program.");
            write_log(ERROR);
            return EXIT_FAILURE;
        }

        close(stdin_pipe[1]); /* finished writing to spawn */

        /* instead of waiting for the cgi script running, we will
         * add it to the select pool here. So once the script finishes
         * running, the OS will be notified to redirect the message back
         * to the client. But need to keep track of the client socket. */
        for (i = 0; i < FD_SETSIZE; i++)
        {
            if (p->clientfd[i] < 0)
            {
                p->clientfd[i] = stdout_pipe[0];
                p->cliaddr[i] = NULL;
                p->clientcgi[i] = client_sock;
                if (ssl_context != NULL)
                    p->httpscontext[i] = ssl_context;

                FD_SET(stdout_pipe[0], &p->read_set);
                p->maxfd =
                        (stdout_pipe[0] > p->maxfd) ? stdout_pipe[0] : p->maxfd;
                p->maxi = (i > p->maxi) ? i : p->maxi;
                break;
            }
        }
        /* in case when there are too many clients */
        if (i == FD_SETSIZE)
            return EXIT_FAILURE;

        return EXIT_SUCCESS;

    }
    /*************** END FORK **************/

    sprintf(log_msg, "Process exiting, badly...how did we get here!?");
    write_log(ERROR);
    return EXIT_FAILURE;
}

