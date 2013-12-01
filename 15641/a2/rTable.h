#ifndef RTABLE_H_
#define RTABLE_H_

#define MAX_NODE 20
#define MAX_OBJECTNUM 20
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
}packet_T;

int32_t NodeList[MAX_NODE];
int32_t NodeAmount;

double matrix[MAX_NODE][MAX_NODE];
int32_t previous[MAX_NODE];
double dist[MAX_NODE];

char objectList[MAX_NODE][MAX_OBJECTNUM][128];
int32_t objectCount[MAX_NODE];

void updateRTable(packet_T newPacket);
void initialize();
int32_t getNodeinMatrix(int32_t Nodeid);
void removeNode(int32_t Nodeid);

void compute();
int32_t getNextHopwithObject(char *objectName);
int32_t getNextHop(int32_t nodeid);
#endif /* RTABLE_H_ */
