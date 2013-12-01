#ifndef __LISODLIB_H__
#define __LISODLIB_H__

#define MAX_HEADER 8192

typedef struct
{
    int content_length;
    char *content_type;
    char *last_modified;
    char *body;
} entity;

typedef struct
{
    char method[MAX_HEADER];
    char path[MAX_HEADER];
    char version[MAX_HEADER];
    char tokens[MAX_HEADER];
    char connection[MAX_HEADER];
    int content_length;
    char body[MAX_HEADER];
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
} pool;

char *root;
int http_port;

char *generate_type(char *filepath);
char *generate_statusline(int statuscode);
int process_file(char path[], entity *entity);
char *formalize_response(int statuscode, entity *entity, int is_alive);
ssize_t send_data(int fd, void *usrbuf, size_t n);
char *parse_value(char request[], char target[]);
int process_request(int client_sock, char header[], request *req);

#endif /* __LISODLIB_H__ */
