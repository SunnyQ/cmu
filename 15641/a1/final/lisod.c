/*
 * This serves as the main file for the lisod server running.
 * The lisod server can hold static website, interact with SSL
 * communication and process cgi requests.The lisod server runs
 * in daemon mode and dump any debug, error and warning messages
 * to an output file.
 *
 *      Author: Yang Sun (yksun@cs.cmu.edu)
 */

#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include "lisodlib.h"
#include "lisodlog.h"
#include "errno.h"

int close_socket(int sock)
{
    if (close(sock))
    {
        sprintf(log_msg, "Failed closing socket... Closing the server...");
        write_log(ERROR);
        return 1;
    }
    return 0;
}

void init_pool(pool *p)
{
    int i;

    p->maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++)
    {
        /* unavailable client file descriptor is marked as -1 */
        p->clientfd[i] = -1;
        /* initially, no ssl context, recognize as http connection */
        p->httpscontext[i] = NULL;
        /* initially, no cgi pipe */
        p->clientcgi[i] = -1;
        /* initially no addr info */
        p->cliaddr[i] = NULL;
        /* clear the client buffer */
        memset(p->clibuf[i], 0, MAX_HEADER);
    }

    /* sock is the only member of select read set */
    FD_ZERO(&p->read_set);
    FD_SET(server_sock, &p->read_set);
    FD_SET(https_sock, &p->read_set);
    p->maxfd = (https_sock > server_sock) ? https_sock : server_sock;
}

int add_client(int client_sock, pool *p, SSL *client_context,
        struct sockaddr_in *addr)
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
            p->cliaddr[i] = addr;
            if (client_context != NULL)
                p->httpscontext[i] = client_context;

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

void clear_sock(pool *p, int i)
{
    close_socket(p->clientfd[i]);
    FD_CLR(p->clientfd[i], &p->read_set);
    p->clientfd[i] = -1;
    p->clientcgi[i] = -1;
    if (p->httpscontext[i] != NULL)
    {
        SSL_shutdown(p->httpscontext[i]);
        SSL_free(p->httpscontext[i]);
        p->httpscontext[i] = NULL;
    }
}

void check_clients(pool *p)
{
    int i, client_sock, cnt, totalsize;
    ssize_t readret;
    char buf[2 * MAX_HEADER]; // in case of super-long request (invalid)
    char header[MAX_HEADER]; // actual request header parsed out from buffer
    char *cur; // hold the current block of request
    char *temp;
    request *req;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        client_sock = p->clientfd[i];
        req = &p->clientreq[i];
        /* if the descriptor is ready, receive it and send it back */
        if ((p->clientfd[i] > 0) && FD_ISSET(p->clientfd[i], &p->ready_set))
        {
            p->nready--;

            /* Deals with CGI output here, redirect the output to client socket */
            if (p->cliaddr[i] == NULL)
            {
                /* Reads the output from cgi script until there is nothing left */
                while ((readret = read(client_sock, buf, MAX_HEADER)) > 0)
                {
                    buf[readret] = '\0';
                    send_data(p->httpscontext[i], p->clientcgi[i], buf,
                            strlen(buf));
                }

                /* Close the stdout pipe once everthing is done */
                close(client_sock);

                /* Reset this slot in pool */
                FD_CLR(client_sock, &p->read_set);
                p->clientfd[i] = -1;
                p->clientcgi[i] = -1;
                p->httpscontext[i] = NULL;
                if (readret == 0)
                {
                    sprintf(log_msg, "CGI spawned process returned with EOF as "
                            "expected.");
                    write_log(DEBUG);
                }
                memset(buf, 0, 2 * MAX_HEADER);
                continue;
            }

            /* Read MAX_HEADER bytes from the client socket, I don't make it
             * in a loop in case of malicious connection. If there are more
             * coming up, buffer the message in this round and combine with the
             * message during the next round. */
            readret =
                    (p->httpscontext[i] == NULL) ?
                            read(p->clientfd[i], buf, MAX_HEADER - 1) :
                            SSL_read(p->httpscontext[i], buf, MAX_HEADER - 1);

            if (readret > 0)
            {
                /* There is message buffered from the previous round; Merge */
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
                    header[cur - buf] = '\0';

                    /* Count the bytes we have read */
                    cnt += cur - buf;

                    /* Parse out Content-Length in order to decide if
                     * there is body part attached */
                    temp = malloc(MAX_HEADER);
                    strcpy(temp, header);
                    char *rv = parse_value(temp, "Content-Length");
                    req->content_length = rv == NULL ? 0 : atoi(rv);

                    /* If Content-Length exists, but no body attached,
                     * return 500 error...
                     * If the content length exceeds the MAX_HEADER, return
                     * a 413 error...
                     */
                    if ((req->content_length >= MAX_HEADER)
                            || (cnt + 4 + req->content_length > totalsize))
                    {
                        /* Clear up the remaining message. We will not process
                         * the rests, because the message followed with a super
                         * long invalid message can possibly be relevant to each
                         * other. Meaningless to process it. If the client
                         * really wants, issue the request again. */
                        if (p->httpscontext[i] == NULL)
                            while (read(p->clientfd[i], buf, MAX_HEADER - 1) > 0)
                                ;
                        else
                            while (SSL_read(p->httpscontext[i], buf,
                                    MAX_HEADER - 1) > 0)
                                ;

                        temp = formalize_response(
                                (req->content_length >= MAX_HEADER) ? 413 : 500,
                                NULL, 0);
                        send_data(ssl_lookup(p, client_sock), client_sock, temp,
                                strlen(temp));
                        free(temp);
                        clear_sock(p, i);
                        break;
                    }

                    /* Parse out the body section for POST use */
                    strncpy(req->body, cur + 4, req->content_length);
                    req->body[req->content_length] = '\0';
                    strcpy(buf, cur + 4 + req->content_length);
                    cnt += 4 + req->content_length;
                    free(temp);

                    /* At this point, we get everything we want, go to
                     * process it and give the response; However, valid
                     * request should always less than 8192 bytes. Ignore
                     * invalid ones. */
                    if (strlen(header) <= MAX_HEADER
                            && process_request(p, client_sock, header, req)
                                    != 0)
                    {
                        close_socket(client_sock);
                        FD_CLR(client_sock, &p->read_set);
                        p->clientfd[i] = -1;
                    }
                }
                memset(buf, 0, 2 * MAX_HEADER);
            }
            if (readret == 0)
            {
                /* receiving is done, so close the client socket */
                clear_sock(p, i);
            }
        }

    }
}

int create_server_sock(struct sockaddr_in *addr, int port)
{
    int sock;

    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        sprintf(log_msg, "Failed creating socket on port %d...", port);
        write_log(ERROR);
        return -1;
    }

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) addr, sizeof(*addr)))
    {
        close_socket(sock);
        sprintf(log_msg, "Failed binding socket on port %d...", port);
        write_log(ERROR);
        return -1;
    }

    if (listen(sock, 5))
    {
        close_socket(sock);
        sprintf(log_msg, "Failed listening on socket on port %d...", port);
        write_log(ERROR);
        return -1;
    }

    return sock;
}

int start_run_server()
{
    int client_sock;
    socklen_t cli_size, https_cli_size;
    char *response;
    struct sockaddr_in addr, cli_addr, https_addr, https_cli_addr;
    static pool pool;

    SSL_CTX *ssl_context = NULL;
    SSL *client_context;

    SSL_load_error_strings();
    SSL_library_init();

    /* we want to use TLSv1 only */
    if ((ssl_context = SSL_CTX_new(TLSv1_server_method())) == NULL)
    {
        sprintf(log_msg, "Error creating SSL context.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    /* register private key */
    if (SSL_CTX_use_PrivateKey_file(ssl_context, ssl_key, SSL_FILETYPE_PEM)
            == 0)
    {
        SSL_CTX_free(ssl_context);
        sprintf(log_msg, "Error associating private key.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    /* register public key (certificate) */
    if (SSL_CTX_use_certificate_file(ssl_context, ssl_crt, SSL_FILETYPE_PEM)
            == 0)
    {
        SSL_CTX_free(ssl_context);
        sprintf(log_msg, "Error associating certificate.");
        write_log(ERROR);
        return EXIT_FAILURE;
    }

    /* create the http socket */
    if ((server_sock = create_server_sock(&addr, http_port)) == -1)
        return EXIT_FAILURE;

    /* create the https socket */
    if ((https_sock = create_server_sock(&https_addr, https_port)) == -1)
    {
        SSL_CTX_free(ssl_context);
        return EXIT_FAILURE;
    }

    init_pool(&pool);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        pool.ready_set = pool.read_set;
        if ((pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL,
                NULL)) == -1)
        {
            close_socket(server_sock);
            SSL_CTX_free(ssl_context);
            sprintf(log_msg,
                    "Failed selecting from pool... Closing the server...");
            write_log(ERROR);
            return EXIT_FAILURE;
        }

        /* regular http connection */
        if (FD_ISSET(server_sock, &pool.ready_set))
        {
            cli_size = sizeof(cli_addr);
            if ((client_sock = accept(server_sock,
                    (struct sockaddr *) &cli_addr, &cli_size)) == -1)
            {
                close_socket(server_sock);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg,
                        "Failed accepting connection.. Closing the server...");
                write_log(ERROR);
                return EXIT_FAILURE;
            }

            sprintf(log_msg, "Accepting a new client: %d", client_sock);
            write_log(DEBUG);

            /* add the client to the pool */
            if (add_client(client_sock, &pool, NULL, &cli_addr) != 0)
            {
                close_socket(server_sock);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg,
                        "Failed adding client: Too many clients... Closing the server...");
                write_log(ERROR);

                response = formalize_response(503, NULL, 0);
                send_data(NULL, client_sock, response, strlen(response));
                return EXIT_FAILURE;
            }
        }

        /* https connection */
        if (FD_ISSET(https_sock, &pool.ready_set))
        {
            https_cli_size = sizeof(https_cli_addr);
            if ((client_sock = accept(https_sock,
                    (struct sockaddr *) &https_cli_addr, &https_cli_size))
                    == -1)
            {
                close_socket(https_sock);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg,
                        "Failed accepting connection.. Closing the server...");
                write_log(ERROR);
                return EXIT_FAILURE;
            }

            sprintf(log_msg, "Accepting a new client from SSL: %d",
                    client_sock);
            write_log(DEBUG);

            /************ WRAP SOCKET WITH SSL ************/
            if ((client_context = SSL_new(ssl_context)) == NULL)
            {
                close_socket(client_sock);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg, "Error creating client SSL context");
                write_log(ERROR);
                return EXIT_FAILURE;
            }

            if (SSL_set_fd(client_context, client_sock) == 0)
            {
                close_socket(client_sock);
                SSL_free(client_context);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg, "Error creating client SSL context.");
                write_log(ERROR);
                return EXIT_FAILURE;
            }

            if (SSL_accept(client_context) <= 0)
            {
                sprintf(log_msg,
                        "Error accepting (handshake) client SSL context.");
                write_log(ERROR);
                close_socket(client_sock);
                SSL_free(client_context);
                continue;
            }
            /************ END WRAP SOCKET WITH SSL ************/

            /* add the client to the pool */
            if (add_client(client_sock, &pool, client_context, &https_cli_addr)
                    != 0)
            {
                response = formalize_response(503, NULL, 0);
                send_data(client_context, client_sock, response,
                        strlen(response));
                close_socket(https_sock);
                SSL_CTX_free(ssl_context);
                sprintf(log_msg,
                        "Failed adding client: Too many clients... Closing the server...");
                write_log(ERROR);
                return EXIT_FAILURE;
            }
        }

        /* check each client and process the ready connected descriptor */
        check_clients(&pool);
    }

    close_socket(server_sock);
    close_socket(https_sock);
    SSL_CTX_free(ssl_context);
    return EXIT_SUCCESS;
}

void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGHUP:
        /* rehash the server */
        start_run_server();
        break;
    case SIGTERM:
        /* finalize and shutdown the server */
        close_socket(server_sock);
        sprintf(log_msg, "Server received kill signal and will be shutdown...");
        write_log(DEBUG);
        break;
    default:
        break;
    }
}

/**
 * internal function daemonizing the process
 */
int daemonize(char* lock_file)
{
    /* drop to having init() as parent */
    int i, lfp, pid = fork();
    char str[256] =
    { 0 };
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    setsid();

    for (i = getdtablesize(); i >= 0; i--)
        close(i);

    i = open("/dev/null", O_RDWR);
    dup(i); /* stdout */
    dup(i); /* stderr */
    umask(027);

    lfp = open(lock_file, O_RDWR | O_CREAT | O_EXCL, 0640);

    if (lfp < 0)
        exit(EXIT_FAILURE); /* can not open */

    if (lockf(lfp, F_TLOCK, 0) < 0)
        exit(EXIT_SUCCESS); /* can not lock */

    /* only first instance continues */
    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str)); /* record pid to lockfile */

    signal(SIGCHLD, SIG_IGN); /* child terminate signal */

    signal(SIGHUP, signal_handler); /* hangup signal */
    signal(SIGTERM, signal_handler); /* software termination signal from kill */

    sprintf(log_msg, "Successfully daemonized lisod process, pid %d.",
            getpid());
    write_log(DEBUG);

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    if (argc != 9)
    {
        fprintf(stderr,
                "usage: %s <HTTP port> <HTTPS port> <log file> <lock file> <www folder> "
                        "<CGI folder or script name> <private key file> <certificate file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    http_port = atoi(argv[1]);
    https_port = atoi(argv[2]);
    log_file = argv[3];
    lock_file = argv[4];
    root = argv[5];
    cgi_root = argv[6];
    ssl_key = argv[7];
    ssl_crt = argv[8];

//    daemonize(lock_file);

    sprintf(log_msg, "Server is starting...");
    if (write_log(DEBUG) == E_FILEOPEN)
    {
        fprintf(stderr, "Failed to start log engine... Closing the server\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "----- Echo Server -----\n");

    return start_run_server();
}
