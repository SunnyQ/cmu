/******************************************************************************
 * lisod.c                                                                     *
 *                                                                             *
 * Description: This file contains the C source code for a select-based web    *
 * 				server. The server can hold a static website and process GET,  *
 * 				POST AND HEAD requests. The server supports pipeline requests  *
 * 				as needed. It can handle multiple clients at once and support  *
 * 				concurrent clients.                                            *
 *                                                                             *
 * Author: Yang Sun <yksun@cs.cmu.edu>                                         *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "lisodlib.h"
#include "lisodlog.h"
#include "errno.h"

int close_socket(int sock)
{
    if (close(sock))
    {
        write_log(ERROR, "Failed closing socket... Closing the server...");
        return 1;
    }
    return 0;
}

void init_pool(int sock, pool *p)
{
    int i;

    p->maxi = -1;
    /* unavailable client file descriptor is marked as -1 */
    for (i = 0; i < FD_SETSIZE; i++)
    {
        p->clientfd[i] = -1;
        memset(p->clibuf[i], 0, MAX_HEADER);
    }

    /* sock is the only member of select read set */
    p->maxfd = sock;
    FD_ZERO(&p->read_set);
    FD_SET(sock, &p->read_set);
}

int add_client(int client_sock, pool *p)
{
    int i, x;
    p->nready--;

    /* set socket into nonblocking mode */
    x = fcntl(client_sock, F_GETFL, 0);
    fcntl(client_sock, F_SETFL, x | O_NONBLOCK);

    for (i = 0; i < FD_SETSIZE; i++)
    {
        if (p->clientfd[i] < 0)
        {
            /* add connected descriptor to the pool */
            p->clientfd[i] = client_sock;

            /* add the descriptor to the descriptor set */
            FD_SET(client_sock, &p->read_set);

            /* update max descriptor and pool high water mark */
            p->maxfd = (client_sock > p->maxfd) ? client_sock : p->maxfd;
            p->maxi = (i > p->maxi) ? i : p->maxi;
            break;
        }
    }
    /* in case when there are too many clients */
    if (i == FD_SETSIZE)
        return 1;
    return 0;
}

int check_clients(pool *p)
{
    int i, client_sock, cnt, totalsize;
    ssize_t readret;
    char buf[2 * MAX_HEADER];   // in case of super-long request (invalid)
    char header[MAX_HEADER];    // actual request header parsed out from buffer
    char *cur;  // hold the current block of request
    char *temp;
    request *req;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        client_sock = p->clientfd[i];
        req = &p->clientreq[i];
        /* if the descriptor is ready, receive it and send it back */
        if ((client_sock > 0) && FD_ISSET(client_sock, &p->ready_set))
        {
            p->nready--;
            readret = 0;
            if ((readret = recv(client_sock, buf, MAX_HEADER, 0)) >= 1)
            {
                /* If client buf has content from previous request,
                 * combine it with the buffer received this time. This is
                 * why we need to allocate more space for the buf. */
                if (strlen(p->clibuf[i]) > 0)
                {
                    temp = malloc(MAX_HEADER);
                    strcpy(temp, buf);
                    sprintf(buf, "%s%s", p->clibuf[i], temp);
                    memset(p->clibuf[i], 0, MAX_HEADER);
                    free(temp);
                }

                cnt = 0;
                totalsize = strlen(buf);

                /* At this point, we should have the complete buffer we
                 * need, then process it. */
                while (cnt < totalsize)
                {
                    /* Parse out each block of the request delimited by
                     * \r\n\r\n according to the RFC spec */
                    cur = strstr(buf, "\r\n\r\n");

                    /* Deal with incomplete request header, save it into
                     * clibuf for the next round */
                    if (cur == NULL)
                    {
                        strcpy(p->clibuf[i], buf);
                        break;
                    }

                    /* Parse out the complete header */
                    strncpy(header, buf, cur - buf);

                    /* Count the bytes we have read */
                    cnt += cur - buf;

                    /* Parse out Content-Length in order to decide if
                     * there is body part attached */
                    temp = malloc(MAX_HEADER);
                    strcpy(temp, header);
                    char *rv = parse_value(temp, "Content-Length");
                    req->content_length = rv == NULL ? 0 : atoi(rv);

                    /* If Content-Length exists, but no body attached,
                     * return 500 error */
                    if (cnt + 4 + req->content_length > totalsize)
                    {
                        temp = formalize_response(500, NULL, 0);
                        send_data(client_sock, temp, strlen(temp));
                        break;
                    }

                    /* Parse out the body section for POST use */
                    strncpy(req->body, cur + 4, req->content_length);
                    strcpy(buf, cur + 4 + req->content_length);
                    cnt += 4 + req->content_length;
                    free(temp);

                    /* At this point, we get everything we want, go to
                     * process it and give the response; However, valid
                     * request should always less than 8192 bytes. Ignore
                     * invalid ones. */
                    if (strlen(header) <= MAX_HEADER
                            && process_request(client_sock, header, req) != 0)
                    {
                        if (close_socket(client_sock))
                            return 1;
                        FD_CLR(client_sock, &p->read_set);
                        p->clientfd[i] = -1;
                    }
                }
                memset(buf, 0, MAX_HEADER);
            }
            if (readret == 0)
            {
                /* receiving is done, so close the client socket */
                if (close_socket(client_sock))
                    return 1;
                FD_CLR(client_sock, &p->read_set);
                p->clientfd[i] = -1;
            }
        }

    }

    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    static pool pool;

    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <HTTP port> <log file> <www folder>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    http_port = atoi(argv[1]);
    log_file = argv[2];
    root = argv[3];

    if (write_log(DEBUG, "Server is starting...") == E_FILEOPEN)
    {
        fprintf(stderr, "Failed to start log engine... Closing the server\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "----- Echo Server -----\n");

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        write_log(ERROR, "Failed creating socket... Closing the server...");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(http_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        write_log(ERROR, "Failed binding socket... Closing the server...");
        return EXIT_FAILURE;
    }

    if (listen(sock, 5))
    {
        close_socket(sock);
        write_log(ERROR, "Failed listening on socket... Closing the server...");
        return EXIT_FAILURE;
    }

    init_pool(sock, &pool);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        pool.ready_set = pool.read_set;
        if ((pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL,
                NULL)) == -1)
        {
            close_socket(sock);
            write_log(ERROR,
                    "Failed selecting from pool... Closing the server...");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(sock, &pool.ready_set))
        {
            cli_size = sizeof(cli_addr);
            if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                    &cli_size)) == -1)
            {
                close_socket(sock);
                write_log(ERROR,
                        "Failed accepting connection.. Closing the server...");
                return EXIT_FAILURE;
            }

            log_fp = fopen(log_file, "a");
            fprintf(log_fp, "%s\t%s\tAccepting a new client: %d\n",
                    current_time(), DEBUG, client_sock);
            fclose(log_fp);

            /* add the client to the pool */
            if (add_client(client_sock, &pool) != 0)
            {
                close_socket(sock);
                // TODO: send some error code to client??
                write_log(ERROR,
                        "Failed adding client: Too many clients... Closing the server...");
                return EXIT_FAILURE;
            }
        }

        /* check each client and process the ready connected descriptor */
        if (check_clients(&pool) != 0)
        {
            close_socket(sock);
            return EXIT_FAILURE;
        }
    }

    close_socket(sock);
    return EXIT_SUCCESS;
}
