/******************************************************************************
 * routed.c                                                                    *
 *                                                                             *
 * Description: This file contains the C source code for a select-based        *
 *              routing daemon server. The server writes back response         *
 *              according to the incoming requests from flaskr web server.     *
 *              It can handle multiple flaskr server at once and support       *
 *              concurrent requests. Meanwhile, it communicates with other     *
 *              peers to update changes.                                       *
 *                                                                             *
 * Author: Yang Sun <yksun@cs.cmu.edu>                                         *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "routed.h"
#include "ospf.h"
#include "rTable.h"

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

void init_pool(pool *p)
{
    int i;

    p->maxi = -1;
    /* unavailable client file descriptor is marked as -1 */
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1;

    /* sock is the only member of select read set */
    FD_ZERO(&p->read_set);
    FD_SET(flaskr_sock, &p->read_set);
    FD_SET(routed_sock, &p->read_set);
    p->maxfd = (flaskr_sock > routed_sock) ? flaskr_sock : routed_sock;
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
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void check_clients(pool *p)
{
    int i, client_sock;
    ssize_t readret;
    char buf[BUF_SIZE];

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        client_sock = p->clientfd[i];
        /* if the descriptor is ready, receive it and send it back */
        if ((client_sock > 0) && FD_ISSET(client_sock, &p->ready_set))
        {
            p->nready--;
            readret = 0;
            while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) > 0)
            {
                if (process_request(client_sock, buf) == EXIT_FAILURE)
                    fprintf(stderr,
                            "Error processing request from client socket: %s\n",
                            buf);
                memset(buf, 0, BUF_SIZE);
            }

            if (readret == 0)
            {
                close_socket(client_sock);
                FD_CLR(client_sock, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

ssize_t send_data(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        nwritten = send(fd, bufp, nleft, 0);
        if (nwritten > 0)
        {
            nleft -= nwritten;
            bufp += nwritten;
        }
    }
    return n;
}

int process_request(int client_sock, char *buf)
{
    request *req;
    response *res;
    file_entry *file_e;
    neightbor_entry *conf_e;
    int scanret, nexthopId;

    res = malloc(sizeof(response));
    req = malloc(sizeof(request));

    scanret = sscanf(buf, "%s %d %s %d %s", req->type, &req->objlen, req->obj,
            &req->pathlen, req->path);
    /* valid GETRD request */
    if (scanret == 3 && strlen(req->type) == 5
            && strncmp(req->type, "GETRD", 5) == 0
            && req->objlen == strlen(req->obj) && req->objlen)
    {
        file_e = objlookup(req->obj);
        /* look up in the local directory */
        if (file_e != NULL )
        {
            conf_e = conflookup(nodeID);
            res->status = "OK";
            sprintf(res->msg, "http://%s:%d%s", conf_e->hostname,
                    conf_e->serverport, file_e->path);
        }
        else
        {
            /* search for the next hop */
            if ((nexthopId = getNextHopwithObject(req->obj)) != -1)
            {
                conf_e = conflookup(nexthopId);
                res->status = "OK";
                sprintf(res->msg, "http://%s:%d/rd/%d?object=%s",
                        conf_e->hostname, conf_e->serverport, conf_e->localport,
                        req->obj);
            }
            /* the file really doesn't exist... */
            else
            {
                res->status = "NOTFOUND";
                strcpy(res->msg, "");

            }
        }
    }
    /* valid ADDFILE request */
    else if (scanret == 5 && strlen(req->type) == 7
            && strncmp(req->type, "ADDFILE", 7) == 0
            && req->pathlen == strlen(req->path) && req->pathlen)
    {
        file_e = objlookup(req->obj);
        if (file_e == NULL && add_objpath(req->obj, req->path) == EXIT_SUCCESS)
        {
            res->status = "OK";
            strcpy(res->msg, "");

            /* broadcast the changes */
            outgoingPacket = generate_self_LSA(++sequenceNum, 0);
            broadcast_neighbor_except(outgoingPacket, -1);
        }
        else
        {
            res->status = "ERROR";
            if (file_e != NULL )
                sprintf(res->msg, "The same object \"%s\" already exists",
                        req->obj);
            else
                sprintf(res->msg, "Failed to add file record for \"%s\"",
                        req->obj);
        }
    }
    /* other invalid requests */
    else
    {
        res->status = "ERROR";
        sprintf(res->msg, "The command \"%s\" is not found or bad format",
                req->type);
    }

    sprintf(res->data, "%s %zd %s", res->status, strlen(res->msg), res->msg);
    send_data(client_sock, res->data, strlen(res->data));

    free(req);
    free(res);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    int client_sock, i;
    socklen_t flaskr_cli_size, routed_cli_size;
    struct sockaddr_in flaskr_addr, flaskr_cli_addr;
    struct sockaddr_in routed_addr, routed_cli_addr;
    static pool pool;
    neightbor_entry *conf_e;
    packet *lsa;

    if (argc != 8)
    {
        fprintf(stderr, "usage: %s <nodeid> <config file> <file list> <adv "
                "cycle time> <neighbor timeout> <retran timeout> "
                "<LSA timeout>\n", argv[0]);
        return EXIT_FAILURE;
    }

    nodeID = atoi(argv[1]);
    neighbor_file = argv[2];
    file_list = argv[3];
    cycle_time = atoi(argv[4]);
    neighbor_timeout = atoi(argv[5]);
    retran_timeout = atoi(argv[6]);
    lsa_timeout = atoi(argv[7]);

    if (retran_timeout >= cycle_time)
    {
        fprintf(stderr,
                "Retransmission time can't be greater than cycle time.\n");
        return EXIT_FAILURE;
    }

    if (lsa_timeout <= cycle_time || neighbor_timeout <= cycle_time)
    {
        fprintf(stderr,
                "LSA timeout and neighbor timeout can't be less than cycle time.\n");
        return EXIT_FAILURE;
    }

    if (init_conf() == -1)
    {
        fprintf(stderr, "Failed initializing configuration file.\n");
        return EXIT_FAILURE;
    }

    if (init_objs() == -1)
    {
        fprintf(stderr, "Failed initializing file list.\n");
        return EXIT_FAILURE;
    }

    /* initialize the routing table */
    initialize();

    /* initialize the lsa array */
    for (i = 0; i < NUM_ENTRIES; i++)
        lsas[i] = NULL;
    sequenceNum = 0;

    if ((conf_e = conflookup(nodeID)) == NULL )
    {
        fprintf(stderr, "Configuration file may be corrupted.\n"
                "It must contain nodeID: %d\n", nodeID);
        return EXIT_FAILURE;
    }

    /* all networked programs must create a socket */
    if ((flaskr_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating flaskr socket.\n");
        return EXIT_FAILURE;
    }

    memset(&flaskr_addr, '\0', sizeof(flaskr_addr));
    flaskr_addr.sin_family = AF_INET;
    flaskr_addr.sin_port = htons(conf_e->localport);
    flaskr_addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(flaskr_sock, (struct sockaddr *) &flaskr_addr,
            sizeof(flaskr_addr)))
    {
        close_socket(flaskr_sock);
        fprintf(stderr, "Failed binding flaskr socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(flaskr_sock, 5))
    {
        close_socket(flaskr_sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /* create the socket for communication among routeds */
    if ((routed_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        close_socket(flaskr_sock);
        fprintf(stderr, "Failed creating routed socket.\n");
        return EXIT_FAILURE;
    }

    memset(&routed_addr, '\0', sizeof(routed_addr));
    routed_addr.sin_family = AF_INET;
    routed_addr.sin_port = htons(conf_e->routingport);
    routed_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(routed_sock, (struct sockaddr *) &routed_addr,
            sizeof(routed_addr)))
    {
        close_socket(flaskr_sock);
        close_socket(routed_sock);
        fprintf(stderr, "Failed binding routed socket.\n");
        return EXIT_FAILURE;
    }

    fcntl(routed_sock, F_SETFL, fcntl(routed_sock, F_GETFL, 0) | O_NONBLOCK);

    init_pool(&pool);

    printf("routing daemon is running...\n");

    timeout.tv_sec = 1;
    cur_time = 0;
    check_point = 0;
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        pool.ready_set = pool.read_set;
        if ((pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL,
                &timeout)) == -1)
        {
            close_socket(flaskr_sock);
            close_socket(routed_sock);
            fprintf(stderr, "Error selecting from pool.\n");
            return EXIT_FAILURE;
        }

        /* OK, now timeout... may need to broadcast */
        if (pool.nready == 0)
        {
            cur_time++;

            /* increment neighbor timeout */
            for (i = 0; i < NUM_ENTRIES; i++)
                if (neighbors[i] != NULL && neighbors[i]->nodeID != nodeID)
                    neighbors[i]->timeout_count++;

            /* increment LSA TTL */
            for (i = 0; i < NUM_ENTRIES; i++)
                if (lsas[i] != NULL && lsas[i]->SenderNodeID != nodeID)
                    lsas[i]->TTL++;

            /* reach the cycle time */
            if (cur_time == cycle_time)
            {
                printf("[DEBUG]: cycle time reaches.\n");
                /* check for down neighbors */
                for (i = 0; i < NUM_ENTRIES; i++)
                {
                    if (neighbors[i] != NULL
                            && neighbors[i]->timeout_count >= neighbor_timeout)
                    {
                        printf("[DEBUG]: neighbor times out.\n");
                        removeNode(neighbors[i]->nodeID);
                        outgoingPacket = generate_self_LSA(++sequenceNum, 0);
                        outgoingPacket.SenderNodeID = neighbors[i]->nodeID;
                        outgoingPacket.TTL = 0;
                        broadcast_neighbor_except(outgoingPacket,
                                neighbors[i]->nodeID);
                        neighbors[i] = NULL;
                    }
                }

                /* check for expired LSAs */
                for (i = 0; i < NUM_ENTRIES; i++)
                {
                    if (lsas[i] != NULL && lsas[i]->TTL >= lsa_timeout)
                    {
                        printf("[DEBUG]: LSA times out.\n");
                        removeNode(lsas[i]->SenderNodeID);
                        lsas[i] = NULL;
                    }
                }

                /* regular broadcasting */
                cur_time = 0;
                outgoingPacket = generate_self_LSA(++sequenceNum, 0);
                broadcast_neighbor_except(outgoingPacket, -1);
            }
            /* retransmit timeout reaches */
            else if (cur_time == check_point)
            {
                retry_transmission(backupPacket);
                check_point = cur_time + retran_timeout;
            }
            timeout.tv_sec = 1;
            continue;
        }

        if (FD_ISSET(flaskr_sock, &pool.ready_set))
        {
            flaskr_cli_size = sizeof(flaskr_cli_addr);
            if ((client_sock = accept(flaskr_sock,
                    (struct sockaddr *) &flaskr_cli_addr, &flaskr_cli_size))
                    == -1)
            {
                close_socket(flaskr_sock);
                close_socket(routed_sock);
                fprintf(stderr, "Error accepting connection.\n");
                return EXIT_FAILURE;
            }

            /* add the client to the pool */
            if (add_client(client_sock, &pool) != EXIT_SUCCESS)
            {
                close_socket(flaskr_sock);
                close_socket(routed_sock);
                fprintf(stderr, "Error adding client: Too many clients.\n");
                return EXIT_FAILURE;
            }
        }

        /* message sent from other routing daemon */
        if (FD_ISSET(routed_sock, &pool.ready_set))
        {
            pool.nready--;
            routed_cli_size = sizeof(routed_cli_addr);
            int readret;
            while ((readret = recvfrom(routed_sock, &incomingPacket,
                    sizeof(incomingPacket), 0,
                    (struct sockaddr *) &routed_cli_addr, &routed_cli_size)) > 0)
            {
                /* receive TTL=0, remove */
                if (incomingPacket.TTL == '0')
                {
                    printf("[DEBUG]: received a packet of TTL=0.\n");
                    removeNode(incomingPacket.SenderNodeID);
                }

                /* ACK received */
                else if (incomingPacket.Type[0] == '1')
                {
                    printf("[DEBUG]: received an ACK packet.\n");
                    neightbor_entry *cur_entry;
                    if ((cur_entry = conflookup(incomingPacket.SenderNodeID))
                            != NULL )
                        cur_entry->isAcked = 1;
                    else
                        fprintf(stderr,
                                "How can we reach here??? ACK from unknown source.\n");
                }

                /* other messages from neighbors */
                else if ((conf_e = ipportlookup(
                        inet_ntoa(routed_cli_addr.sin_addr),
                        ntohs(routed_cli_addr.sin_port))) != NULL )
                {
                    /* reset the neighbor timeout */
                    conf_e->timeout_count = 0;

                    /* receive my own annoucement */
                    if (incomingPacket.SenderNodeID == nodeID
                            && incomingPacket.seq_num > sequenceNum)
                    {
                        printf("[DEBUG]: received my own packet, "
                                "adjust sequence number\n");
                        sequenceNum = incomingPacket.seq_num + 1;
                        backupPacket.seq_num = sequenceNum;
                    }

                    /* useful LSA */
                    else if (incomingPacket.SenderNodeID != nodeID)
                    {
                        /* look up the lsa from local store */
                        if ((lsa = lsalookup(incomingPacket.SenderNodeID))
                                == NULL )
                        {
                            printf(
                                    "[DEBUG]: new LSA received, initialize...\n");
                            lsa = lsainit(incomingPacket.SenderNodeID);
                        }

                        /* LSA is from neighbor */
                        if (conf_e->nodeID == incomingPacket.SenderNodeID)
                        {
                            printf("[DEBUG]: neighbor's LSA received\n");
                            /* if receive lower sequence number, send it back */
                            if (incomingPacket.seq_num < lsa->seq_num + 1)
                            {
                                printf(
                                        "[DEBUG]: received a packet whose "
                                                "sequence number is smaller than expected.\n");
                                incomingPacket.seq_num = lsa->seq_num;
                                send_UDP_packet(conf_e, incomingPacket);
                                continue;
                            }

                            /* valid LSA reaches here, respond ACK */
                            outgoingPacket = generate_self_LSA(
                                    incomingPacket.seq_num, 1);
                            send_UDP_packet(conf_e, outgoingPacket);
                        }

                        /* valid LSA reaches here, update local store */
                        lsaupdate(lsa, incomingPacket);

                        /* update routing table */
                        packet_T tempPacket;
                        tempPacket.SenderNodeID = incomingPacket.SenderNodeID;
                        tempPacket.seq_num = incomingPacket.seq_num;
                        tempPacket.numLinkEntries =
                                incomingPacket.numLinkEntries;
                        tempPacket.numObjectEntries =
                                incomingPacket.numObjectEntries;
                        memcpy(tempPacket.entries, incomingPacket.entries,
                                NUM_ENTRIES * 132);
                        updateRTable(tempPacket);

                        /* continue broadcast the changes */
                        incomingPacket.TTL--;
                        broadcast_neighbor_except(incomingPacket,
                                conf_e->nodeID);
                    }

                }
            }
        }

        check_clients(&pool);
    }

    close_socket(flaskr_sock);
    close_socket(routed_sock);

    return EXIT_SUCCESS;
}
