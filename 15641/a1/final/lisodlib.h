/*
 * This serves as the header file of the lisod
 * backend library.
 *
 *      Author: Yang Sun (yksun@cs.cmu.edu)
 */


#ifndef __LISODLIB_H__
#define __LISODLIB_H__

/******** MACRO variable **********/
#define MAX_HEADER 8192
#define BUF_SIZE 512
/******** MACRO variable **********/

/******** struct definition *******/
typedef struct
{
    int content_length;
    char *content_type;
    char *last_modified;
    char *body;
} entity;

typedef struct
{
    char method[BUF_SIZE];
    char uri[BUF_SIZE];
    char version[BUF_SIZE];
    char tokens[BUF_SIZE];
    char connection[BUF_SIZE];
    int content_length;
    char content_type[BUF_SIZE];
    char path_info[BUF_SIZE];
    char query_str[BUF_SIZE];
    char body[MAX_HEADER]; // the size of the body can't exceed 8k
    char remote_addr[BUF_SIZE];
    char http_accept[BUF_SIZE];
    char http_referer[BUF_SIZE];
    char http_accept_encoding[BUF_SIZE];
    char http_accept_language[BUF_SIZE];
    char http_accept_charset[BUF_SIZE];
    char http_host[BUF_SIZE];
    char http_cookie[BUF_SIZE];
    char http_user_agent[BUF_SIZE];
    char server_port[BUF_SIZE];
} request;

typedef struct
{
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    request clientreq[FD_SETSIZE];
    int clientfd[FD_SETSIZE];
    char clibuf[FD_SETSIZE][MAX_HEADER];
    SSL *httpscontext[FD_SETSIZE];
    struct sockaddr_in *cliaddr[FD_SETSIZE];
    int clientcgi[FD_SETSIZE];
} pool;
/******** struct definition *******/

/******** global variable **********/
char *lock_file;
int server_sock;
int https_sock;
char *root;
char *cgi_root;
int http_port;
int https_port;
char *ssl_key;
char *ssl_crt;
/******** global variable **********/

/******** function header **********/
SSL *ssl_lookup(pool *p, int fd);
char *generate_type(char *filepath);
char *generate_statusline(int statuscode);
int process_file(char path[], entity *entity);
char *formalize_response(int statuscode, entity *entity, int is_alive);
ssize_t send_data(SSL *ssl_context, int fd, void *usrbuf, size_t n);
char *parse_value(char request[], char target[]);
void clear_mem(request *req);
int process_request(pool *p, int client_sock, char header[], request *req);
int run_cgi(pool *p, int client_sock, char header[], request *req);
void execve_error_handler();
/******** function header **********/

#endif /* __LISODLIB_H__ */
