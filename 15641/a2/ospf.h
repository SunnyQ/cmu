/*
 * ospf.h
 *
 *  Created on: Oct 24, 2012
 *      Author: kobe
 */

#ifndef OSPF_H_
#define OSPF_H_

#define EXIT_SUCCESS 0
#define BUF_SIZE 4096
#define NUM_ENTRIES 1024

typedef struct
{
    char version;
    char TTL;
    char Type[2];
    int32_t SenderNodeID;
    int32_t seq_num;
    int32_t numLinkEntries;
    int32_t numObjectEntries;
    char entries[NUM_ENTRIES * 132];
} packet;

typedef struct
{
    int32_t nodeID;
    char hostname[32];
    int routingport;
    int localport;
    int serverport;
    int isdown;
    int isAcked;
    struct sockaddr_in saddr;
    int timeout_count;
} neightbor_entry;

typedef struct
{
    char object[128];
    char path[1024];
} file_entry;

int32_t nodeID;
int routed_sock;
char *neighbor_file;
char *file_list;
int cycle_time, neighbor_timeout, retran_timeout, lsa_timeout;
neightbor_entry *self_conf;
neightbor_entry *neighbors[NUM_ENTRIES];
file_entry *files[NUM_ENTRIES];
packet *lsas[NUM_ENTRIES];
packet incomingPacket, outgoingPacket, backupPacket;
struct timeval timeout;
int cur_time, check_point;
int sequenceNum;

int init_conf();
int init_objs();
int add_objpath(char *obj, char *path);
file_entry *objlookup(char *obj);
neightbor_entry *conflookup(int32_t nodeID);
neightbor_entry *ipportlookup(char *ip, int port);
packet *lsainit(int32_t nodeID);
void lsaupdate(packet *p1, packet p2);
packet *lsalookup(int32_t nodeID);
int send_UDP_packet(neightbor_entry *conf, packet p);
int broadcast_neighbor_except(packet p, int32_t nodeID);
int retry_transmission(packet p);
packet generate_self_LSA(int sequenceNum, int isAck);
int getPacketSize(packet p);

#endif /* OSPF_H_ */
