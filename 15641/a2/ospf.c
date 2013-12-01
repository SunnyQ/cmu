/******************************************************************************
 * ospf.c                                                                      *
 *                                                                             *
 * Description: This file contains the logic operations of ospf actions.       *
 *                                                                             *
 * Author: Yang Sun <yksun@cs.cmu.edu>                                         *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "ospf.h"

int init_conf()
{
    FILE *fp;
    int i;
    char buf[BUF_SIZE];
    struct hostent *he;
    struct in_addr **addr_list;

    if (!(fp = fopen(neighbor_file, "r")))
    {
        fprintf(stderr, "No such file: %s\n", neighbor_file);
        return EXIT_FAILURE;
    }

    /* Initially, empty confs array */
    for (i = 0; i < NUM_ENTRIES; i++)
        neighbors[i] = NULL;

    i = 0;
    while (fgets(buf, BUF_SIZE, fp))
    {
        neightbor_entry *conf = malloc(sizeof(neightbor_entry));
        if (sscanf(buf, "%d %s %d %d %d", &conf->nodeID, conf->hostname,
                &conf->routingport, &conf->localport, &conf->serverport) != 5)
        {
            fprintf(stderr, "Configuration file format is invalid: %s\n", buf);
            return EXIT_FAILURE;
        }
        conf->isdown = 0;
        conf->isAcked = 1;
        conf->timeout_count = 0;

        memset(&conf->saddr, '\0', sizeof(conf->saddr));
        conf->saddr.sin_family = AF_INET;
        conf->saddr.sin_port = htons(conf->routingport);

        /* get the real ip address */
        he = gethostbyname(conf->hostname);
        addr_list = (struct in_addr **) he->h_addr_list;

        memset(conf->hostname, 0, 32);
        strcpy(conf->hostname, inet_ntoa(*addr_list[0]));
        if (inet_aton(conf->hostname, &conf->saddr.sin_addr) == 0)
        {
            fprintf(stderr, "Failed calling inet_aton.\n");
            return EXIT_FAILURE;
        }

        neighbors[i++] = conf;

        if (conf->nodeID == nodeID)
            self_conf = conf;
    }

    memset(buf, 0, BUF_SIZE);
    fclose(fp);
    return EXIT_SUCCESS;
}

int init_objs()
{
    FILE *fp;
    int i;
    char buf[BUF_SIZE];

    if (!(fp = fopen(file_list, "r")))
    {
        fprintf(stderr, "No such file: %s\n", file_list);
        return EXIT_FAILURE;
    }

    /* Initially, empty file list array */
    for (i = 0; i < NUM_ENTRIES; i++)
        files[i] = NULL;

    i = 0;
    while (fgets(buf, BUF_SIZE, fp))
    {
        file_entry *file = malloc(sizeof(file_entry));
        if (sscanf(buf, "%s %s", file->object, file->path) != 2)
        {
            fprintf(stderr, "file list file format is invalid: %s\n", buf);
            return EXIT_FAILURE;
        }
        files[i++] = file;
    }

    memset(buf, 0, BUF_SIZE);
    fclose(fp);
    return EXIT_SUCCESS;
}

file_entry *objlookup(char *obj)
{
    int i;
    for (i = 0; i < NUM_ENTRIES; i++)
    {
        if (files[i] != NULL && strcmp(files[i]->object, obj) == 0)
            return files[i];
    }
    return NULL ;
}

neightbor_entry *conflookup(int32_t nodeID)
{
    int i;
    for (i = 0; i < NUM_ENTRIES; i++)
    {
        if (neighbors[i] != NULL && neighbors[i]->nodeID == nodeID)
            return neighbors[i];
    }
    return NULL ;
}

neightbor_entry *ipportlookup(char *ip, int port)
{
    int i;
    for (i = 0; i < NUM_ENTRIES; i++)
    {
        if (neighbors[i] != NULL && strcmp(neighbors[i]->hostname, ip) == 0
                && neighbors[i]->routingport == port)
            return neighbors[i];
    }
    return NULL ;
}

packet *lsalookup(int32_t nodeID)
{
    int i;
    for (i = 0; i < NUM_ENTRIES; i++)
    {
        if (lsas[i] != NULL && lsas[i]->SenderNodeID == nodeID)
            return lsas[i];
    }
    return NULL ;
}

packet *lsainit(int32_t nodeID)
{
    int i;
    packet *savedPacket;

    savedPacket = malloc(sizeof(packet));

    savedPacket->SenderNodeID = nodeID;
    savedPacket->seq_num = -1;

    for (i = 0; i < NUM_ENTRIES; i++)
        if (lsas[i] == NULL )
        {
            lsas[i] = savedPacket;
            return lsas[i];
        }
    return NULL ;
}

void lsaupdate(packet *p1, packet p2)
{
    int i;
    if (p1->SenderNodeID == p2.SenderNodeID && p1->seq_num < p2.seq_num)
    {
        p1->TTL = 0;
        for (i = 0; i < sizeof(p1->entries); i++)
        {
            p1->entries[i] = p2.entries[i];
        }
        p1->numLinkEntries = p2.numLinkEntries;
        p1->numObjectEntries = p2.numObjectEntries;
        p1->seq_num = p2.seq_num;
        p1->version = p2.version;
        p1->Type[0] = p2.Type[0];
        p1->Type[1] = p2.Type[1];
    }
}

int add_objpath(char *obj, char *path)
{
    FILE *fp;
    int i;
    file_entry *file;

    if (!(fp = fopen(file_list, "a")))
        return EXIT_FAILURE;
    fprintf(fp, "%s %s\n", obj, path);
    fclose(fp);

    for (i = 0; i < NUM_ENTRIES; i++)
    {
        if (files[i] == NULL )
        {
            file = malloc(sizeof(file_entry));
            strncpy(file->object, obj, 128);
            strncpy(file->path, path, 1024);
            files[i] = file;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int send_UDP_packet(neightbor_entry *conf, packet p)
{
    if (conf->nodeID == nodeID)
        return EXIT_SUCCESS;

    if (sendto(routed_sock, &p, getPacketSize(p), 0,
            (struct sockaddr *) &conf->saddr, sizeof(conf->saddr)) == -1)
    {
        fprintf(stderr, "Failed sending the message.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int getPacketSize(packet p)
{
    int i, cur, total_size;
    total_size = 5 * sizeof(int32_t);
    total_size += 5 * p.numLinkEntries;

    cur = 5 * p.numLinkEntries;
    for (i = 0; i < p.numObjectEntries; i++)
    {
        total_size += strlen(p.entries + cur) + 1;
        cur += strlen(p.entries + cur) + 1;
    }
    return total_size;
}

int broadcast_neighbor_except(packet p, int32_t exception)
{
    int i, ret;

    for (i = 0; i < NUM_ENTRIES; i++)
        if (neighbors[i] != NULL && neighbors[i]->nodeID != exception
                && neighbors[i]->nodeID != nodeID)
        {
            if ((ret = send_UDP_packet(neighbors[i], p)) != 0)
                return ret;
            neighbors[i]->isAcked = 0;
        }

    printf("[DEBUG]: broadcast the message to the neighbors.\n");
    backupPacket = p;
    check_point = cur_time + retran_timeout;
    return EXIT_SUCCESS;
}

int retry_transmission(packet p)
{
    int i, ret;

    for (i = 0; i < NUM_ENTRIES; i++)
        if (neighbors[i] != NULL && neighbors[i]->isAcked == 0)
        {
            if ((ret = send_UDP_packet(neighbors[i], p)) != 0)
            {
                printf("[DEBUG]: Retransmission to neighbor %d.\n",
                        neighbors[i]->nodeID);
                return ret;
            }
        }
    return EXIT_SUCCESS;
}

packet generate_self_LSA(int seqNum, int isAck)
{
    packet outgoingPacket;
    int i, pos;

    memset(&outgoingPacket, '\0', sizeof(outgoingPacket));
    outgoingPacket.SenderNodeID = nodeID;
    outgoingPacket.TTL = 32;
    snprintf(outgoingPacket.Type, 2, "%d", isAck);
    outgoingPacket.seq_num = seqNum;

    if (isAck)
    {
        outgoingPacket.numLinkEntries = 0;
        outgoingPacket.numObjectEntries = 0;
        outgoingPacket.entries[0] = '\0';
    }
    else
    {
        pos = 0;
        outgoingPacket.numLinkEntries = 0;
        for (i = 0; i < NUM_ENTRIES; i++)
            if (neighbors[i] != NULL && neighbors[i]->nodeID != nodeID)
            {
                outgoingPacket.numLinkEntries++;
                snprintf(outgoingPacket.entries + pos, sizeof(int32_t), "%d",
                        neighbors[i]->nodeID);
                pos += sizeof(int32_t);
                outgoingPacket.entries[pos++] = '\0';
            }

        outgoingPacket.numObjectEntries = 0;
        for (i = 0; i < NUM_ENTRIES; i++)
            if (files[i] != NULL )
            {
                outgoingPacket.numObjectEntries++;
                snprintf(outgoingPacket.entries + pos,
                        strlen(files[i]->object) + 1, "%s", files[i]->object);
                pos += strlen(files[i]->object);
                outgoingPacket.entries[pos++] = '\0';
            }
    }

    return outgoingPacket;
}

