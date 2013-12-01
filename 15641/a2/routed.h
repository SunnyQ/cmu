/******************************************************************************
 * rd.h                                                                        *
 *                                                                             *
 * Description: This file contains the header file for a select-based          *
 *              routing daemon server.                                         *
 *                                                                             *
 * Author: Yang Sun <yksun@cs.cmu.edu>                                         *
 *                                                                             *
 *******************************************************************************/

#ifndef ROUTED_H_
#define ROUTED_H_

typedef struct
{
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
} pool;

typedef struct
{
    char type[16];
    int objlen;
    char obj[128];
    int pathlen;
    char path[1024];
} request;

typedef struct
{
    char *status;
    char data[1024];
    char msg[1024];
} response;

int flaskr_sock;

int close_socket(int sock);
void init_pool(pool *p);
int add_client(int client_sock, pool *p);
void check_clients(pool *p);
ssize_t send_data(int fd, void *usrbuf, size_t n);
int process_request(int client_sock, char *buf);
int main(int argc, char* argv[]);

#endif /* ROUTED_H_ */
