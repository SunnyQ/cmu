/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen,
 *          Yang Sun <yksun@cs.cmu.edu>
 *          Fei Xie <fxie@andrew.cmu.edu>
 * Class: 15-641 (Fall 2012)
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"

char **work_queue; // all hash chunks for the current download
linkNode *peers[BT_MAX_PEERS]; // keep info of all the available peers
short jobs[BT_MAX_PEERS]; // the current job running on each peer
int lastACKed[BT_MAX_PEERS]; // for sliding window control, the last packet acked
int lastSent[BT_MAX_PEERS]; // for sliding window control, the last packet sent
int nextExpected[BT_MAX_PEERS]; // for sliding window control, the next expected packet from sender
int TTL[BT_MAX_PEERS][BT_CHUNK_SIZE]; // for reliability control - DATA timeout
int GETTTL[BT_MAX_PEERS]; // for reliability control - GET timeout
int dup_ack[BT_MAX_PEERS]; // for reliability control - increment duplicate acks
int numGetMisses[BT_MAX_PEERS];
int numDataMisses[BT_MAX_PEERS];
char outf[128];
char chunkf[128];
short nodeInMap; // the node the packet received from
int windowSize[BT_MAX_PEERS];
int congestionState[BT_MAX_PEERS];
int ssthresh[BT_MAX_PEERS];
struct timeval startTime;
rttRecord *headRTT;
int numMismatches; //3 dup will invoke retransmission, no need to send more than 3 same ACK

void peer_run(bt_config_t *config);

/* when windowSize changed, record it on a file */
void recordWindowSize(bt_config_t *config)
{
    FILE * pFile;
    char fileName[50];
    struct timeval tvEnd;
    long diff;

    sprintf(fileName, "congestionWindow_%hi", config->identity);
    pFile = fopen(fileName, "a");

    gettimeofday(&tvEnd, NULL );
    diff = (tvEnd.tv_usec + 1000000 * tvEnd.tv_sec)
            - (startTime.tv_usec + 1000000 * startTime.tv_sec);
    fprintf(pFile, "f%d    %ld    %d\n", nodeInMap, diff,
            windowSize[nodeInMap]);

    fclose(pFile);
}

void paramInit(bt_config_t *config, int argc, char **argv)
{
    int i, j;
    struct timeval tvBegin;

    headRTT = NULL;
    gettimeofday(&tvBegin, NULL );
    startTime.tv_sec = tvBegin.tv_sec;
    startTime.tv_usec = tvBegin.tv_usec;

    bt_init(config, argc, argv);

    for (i = 0; i < BT_MAX_PEERS; i++)
    {
        for (j = 0; j < BT_CHUNK_SIZE; j++)
            TTL[i][j] = -1;
        jobs[i] = -1;
        dup_ack[i] = 0;
        GETTTL[i] = -1;
        windowSize[i] = 0;
        congestionState[i] = -1;
        numGetMisses[i] = 0;
        numDataMisses[i] = -1;
    }
    numMismatches = 0;
}

int main(int argc, char **argv)
{
    bt_config_t config;
    paramInit(&config, argc, argv);

    DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

#ifdef TESTING
    config.identity = 1; // your group number here
    strcpy(config.chunk_file, "chunkfile");
    strcpy(config.has_chunk_file, "haschunks");
#endif

    bt_parse_command_line(&config);

#ifdef DEBUG
    if (debug & DEBUG_INIT)
    {
        bt_dump_config(&config);
    }
#endif

    peer_run(&config);
    return 0;
}

void free_chunks(char **chunks, int size)
{
    int i;
    for (i = 0; i < size; i++)
        if (chunks[i] != NULL )
            free(chunks[i]);
    free(chunks);
}

char **has_chunks(bt_config_t *config, packet *p, char **chunk_list)
{
    char **haschunk_list;
    FILE *fp;
    char buf[USERBUF_SIZE];
    char chunk[BT_CHUNKS_SIZE + 1];
    int num_chunks, haschunk_pos;

    haschunk_pos = 0;
    haschunk_list = (char **) malloc(p->payload[0] * sizeof(char *));
    fp = fopen(config->has_chunk_file, "r");
    while (fgets(buf, sizeof(buf), fp))
    {
        sscanf(buf, "%*d %s", chunk);
        for (num_chunks = 0; num_chunks < p->payload[0]; num_chunks++)
            if (strncmp(chunk_list[num_chunks], chunk, BT_CHUNKS_SIZE) == 0)
            {
                haschunk_list[haschunk_pos] = (char *) malloc(
                        BT_CHUNKS_SIZE + 1);
                strcpy(haschunk_list[haschunk_pos++], chunk);
            }
        memset(buf, 0, USERBUF_SIZE);
        memset(chunk, 0, BT_CHUNKS_SIZE + 1);
    }
    fclose(fp);
    return haschunk_list;
}

packet *packet_factory(int type)
{
    packet *p;

    p = (packet *) malloc(sizeof(packet));
    p->magic_num = 15441;
    p->version = 1;
    p->type = type;
    p->header_length = 16;

    return p;
}

void sendback_response(int sock, char **haschunk_list, int size,
        struct sockaddr_in from)
{

    int num_chunks, head;
    packet *p;

    p = packet_factory(IHAVE);
    dprintf(STDOUT_FILENO, "Response payload has: \n");
    for (num_chunks = 0; num_chunks < size; num_chunks++)
    {
        head = 4 + num_chunks * BT_CHUNKS_SIZE;
        strncpy(p->payload + head, haschunk_list[num_chunks], BT_CHUNKS_SIZE);
        dprintf(STDOUT_FILENO, "%s\n", haschunk_list[num_chunks]);
    }

    p->packet_length = 20 + num_chunks * BT_CHUNKS_SIZE;
    p->payload[0] = num_chunks;
    p->payload[1] = '\0';
    p->payload[2] = '\0';
    p->payload[3] = '\0';

    spiffy_sendto(sock, p, p->packet_length, 0, (struct sockaddr *) &from,
            sizeof(from));
    free(p);
}

int getChunkId(char *accChunk, char *filename)
{
    FILE *fp;
    char buf[USERBUF_SIZE];
    char chunk[BT_CHUNKS_SIZE + 1];
    int currentID;

    fp = fopen(filename, "r");
    while (fgets(buf, sizeof(buf), fp))
    {
        sscanf(buf, "%d %s", &currentID, chunk);
        if (strncmp(accChunk, chunk, BT_CHUNKS_SIZE) == 0)
        {
            fclose(fp);
            return currentID;
        }
        memset(buf, 0, USERBUF_SIZE);
        memset(chunk, 0, BT_CHUNKS_SIZE + 1);
    }

    fclose(fp);
    return -1;
}

char *getChunkHash(short id, char *filename)
{
    FILE *fp;
    char buf[USERBUF_SIZE];
    char *chunk;
    int currentID;

    chunk = (char *) malloc((BT_CHUNKS_SIZE + 1) * sizeof(char));
    fp = fopen(filename, "r");
    while (fgets(buf, sizeof(buf), fp))
    {
        sscanf(buf, "%d %s", &currentID, chunk);
        if (currentID == id)
            break;
        memset(buf, 0, USERBUF_SIZE);
        memset(chunk, 0, BT_CHUNKS_SIZE + 1);
    }

    fclose(fp);
    return chunk;
}

void processData(packet *incomingPacket, bt_config_t *config, int sock,
        struct sockaddr_in from)
{
    FILE * outfile;
    packet *p;

    outfile = fopen(outf, "r+b");
    // look for the correct position to insert the packet data
    long int offset = BT_CHUNK_SIZE * BT_FULL_DATASIZE * jobs[nodeInMap]
            + BT_FULL_DATASIZE * (incomingPacket->sequence_num - 1);
    fseek(outfile, offset, SEEK_SET);
    fwrite(incomingPacket->payload, sizeof(char), BT_FULL_DATASIZE, outfile);
    fclose(outfile);

    p = packet_factory(ACK);
    p->packet_length = 16;
    p->ack_num = incomingPacket->sequence_num;
    spiffy_sendto(sock, p, p->packet_length, 0, (struct sockaddr *) &from,
            sizeof(from));
    dprintf(STDOUT_FILENO, "sending ACK num: %d back to %d\n", p->ack_num,
            nodeInMap);
    free(p);
}

void sendData(struct sockaddr_in from, bt_config_t *config)
{
    packet *p;
    FILE *fp;

    if ((fp = fopen("example/C.tar", "rb")) == NULL )
        return;

    p = packet_factory(DATA);
    p->packet_length = 16 + BT_FULL_DATASIZE;
    p->sequence_num = ++lastSent[nodeInMap];

    // look for the correct position to read the data to packet
    long int offset = BT_CHUNK_SIZE * BT_FULL_DATASIZE * jobs[nodeInMap]
            + BT_FULL_DATASIZE * (p->sequence_num - 1);
    fseek(fp, offset, SEEK_SET);
    fread(p->payload, 1, BT_FULL_DATASIZE, fp);
    fclose(fp);

    spiffy_sendto(config->sock, p, p->packet_length, 0,
            (struct sockaddr *) &from, sizeof(from));
    TTL[nodeInMap][p->sequence_num - 1] = 0;
    dprintf(STDOUT_FILENO, "sending data: %d\n", p->sequence_num);
    free(p);
}

int queue_contains(char *chunkHash)
{
    int i;
    for (i = 0; i < BT_MAX_NUM_CHUNKS; i++)
    {
        if (work_queue[i] == NULL )
            continue;
        else if (strcmp(work_queue[i], chunkHash) == 0)
            return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void queue_remove(char *chunkHash)
{
    int i;
    for (i = 0; i < BT_MAX_NUM_CHUNKS; i++)
    {
        if (work_queue[i] == NULL )
            continue;
        else if (strcmp(work_queue[i], chunkHash) == 0)
        {
            work_queue[i] = NULL;
            return;
        }
    }
}

int queue_empty()
{
    int i;
    for (i = 0; i < BT_MAX_NUM_CHUNKS; i++)
        if (work_queue[i] != NULL )
            return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void sendGetSW(int sock, struct sockaddr_in from)
{
    packet *p;

    // check if the target chunkHash has been transferred by other peers
    // allow concurrent download
    while (queue_contains(peers[nodeInMap]->chunkHash) == EXIT_FAILURE)
    {
        if (peers[nodeInMap]->next == NULL )
        {
            if (queue_empty() == EXIT_SUCCESS)
            {
                dprintf(STDOUT_FILENO, "JOB is done\n");
                numDataMisses[nodeInMap] = -1;
            }
            return;
        }

        linkNode *temp = peers[nodeInMap];
        peers[nodeInMap] = peers[nodeInMap]->next;
        free(temp);
    }

    if (peers[nodeInMap]->chunkHash == NULL )
    {
        dprintf(STDOUT_FILENO, "sending chunk equals zero\n");
        return;
    }

    p = packet_factory(GET);
    p->packet_length = 20 + BT_CHUNKS_SIZE;
    p->ack_num = 0;

    p->payload[0] = 1;
    p->payload[1] = '\0';
    p->payload[2] = '\0';
    p->payload[3] = '\0';

    strncpy(p->payload + 4, peers[nodeInMap]->chunkHash, BT_CHUNKS_SIZE);
    spiffy_sendto(sock, p, p->packet_length, 0, (struct sockaddr *) &from,
            sizeof(from));

    jobs[nodeInMap] = getChunkId(peers[nodeInMap]->chunkHash, chunkf);

    dprintf(STDOUT_FILENO, "Requesting chunk ID: %d from %d\n", jobs[nodeInMap],
            nodeInMap);

    nextExpected[nodeInMap] = 1;
    GETTTL[nodeInMap] = 0;
    free(p);

    // chunk is transferring, remove it from work queue so that
    // other peers won't transfer this again
    queue_remove(getChunkHash(jobs[nodeInMap], chunkf));
}

char **retrieve_chunk_list(packet *incomingPacket)
{
    char **chunk_list;
    int num_chunks, head;

    chunk_list = (char **) malloc(incomingPacket->payload[0] * sizeof(char *));

    for (num_chunks = 0; num_chunks < incomingPacket->payload[0]; num_chunks++)
    {
        chunk_list[num_chunks] = (char *) malloc(BT_CHUNKS_SIZE + 1);
        head = 4 + num_chunks * BT_CHUNKS_SIZE;
        strncpy(chunk_list[num_chunks], incomingPacket->payload + head,
                BT_CHUNKS_SIZE);
        chunk_list[num_chunks][BT_CHUNKS_SIZE] = '\0';
    }

    return chunk_list;
}

void allocate_peer_chunks(char **chunk_list, int size)
{
    linkNode *curNode;
    int i;

    peers[nodeInMap] = (linkNode *) malloc(sizeof(linkNode));
    curNode = peers[nodeInMap];
    for (i = 0; i < size; i++) //judgement to be implemented
    {
        strncpy(curNode->chunkHash, chunk_list[i], BT_CHUNKS_SIZE + 1);
        if (i + 1 < size)
        {
            curNode->next = (linkNode *) malloc(sizeof(linkNode));
            curNode = curNode->next;
        }
    }
}

void resetCCRTT() //reset the rtt recorder
{
    rttRecord *curRTT;
    rttRecord *nextRTT;

    for (curRTT = headRTT; curRTT != NULL ; curRTT = nextRTT)
    {
        nextRTT = curRTT->next;
        free(curRTT);
    }
    headRTT = NULL;
}

void congestionControlRetransmit(bt_config_t *config)
{
    int j;
    switch (congestionState[nodeInMap])
    {
    case CA: //congestion avoidance session
        resetCCRTT();
        //no break
    case SLOWSTART: //slow start session
        ssthresh[nodeInMap] = MAX(2, windowSize[nodeInMap] / 2);
        //change working state
        congestionState[nodeInMap] = 0; //slow start
        windowSize[nodeInMap] = 1;
        for (j = 0; j < BT_CHUNK_SIZE; j++) //reset all TTL
            TTL[nodeInMap][j] = -1;
        recordWindowSize(config);
        break;
    }
}

void congestionControlNormal(packet *incomingPacket, bt_config_t *config)
{
    rttRecord *tmp;
    switch (congestionState[nodeInMap])
    {
    case INIT:
        windowSize[nodeInMap] = 1;
        ssthresh[nodeInMap] = 64;
        break;
    case SLOWSTART:
        windowSize[nodeInMap]++;
        if (windowSize[nodeInMap] >= ssthresh[nodeInMap])
            congestionState[nodeInMap] = CA; //slow start state change to congestion state
        recordWindowSize(config);
        break;
    case CA:
        tmp = headRTT;
        if (tmp != NULL && tmp->seq == incomingPacket->ack_num)
        {
            headRTT = tmp->next;
            free(tmp);
            windowSize[nodeInMap]++;
            recordWindowSize(config);
        }
        break;
    }
}

void process_inbound_udp(int sock, bt_config_t *config)
{
    struct sockaddr_in from;
    socklen_t fromlen;
    char **chunk_list;
    char **haschunk_list;
    packet incomingPacket;
    bt_peer_t *curPeer;
    packet *p;
    int i;
    rttRecord *tempRtt;

    fromlen = sizeof(from);
    spiffy_recvfrom(sock, &incomingPacket, sizeof(incomingPacket), 0,
            (struct sockaddr *) &from, &fromlen);

    // check the node where the packet comes from depending on ip and port
    for (curPeer = config->peers; curPeer != NULL ; curPeer = curPeer->next)
        if (strcmp(inet_ntoa(curPeer->addr.sin_addr), inet_ntoa(from.sin_addr))
                == 0 && ntohs(curPeer->addr.sin_port) == ntohs(from.sin_port))
            nodeInMap = curPeer->id;

    switch (incomingPacket.type)
    {
    case WHOHAS:
        // sender
        dprintf(STDOUT_FILENO, "WHOHAS received\n");
        chunk_list = retrieve_chunk_list(&incomingPacket);
        haschunk_list = has_chunks(config, &incomingPacket, chunk_list);
        if (haschunk_list[0] != NULL )
            sendback_response(sock, haschunk_list, incomingPacket.payload[0],
                    from);
        free_chunks(chunk_list, incomingPacket.payload[0]);
        free_chunks(haschunk_list, incomingPacket.payload[0]);
        break;
    case IHAVE:
        // requester
        dprintf(STDOUT_FILENO, "IHAVE received\n");
        chunk_list = retrieve_chunk_list(&incomingPacket);
        allocate_peer_chunks(chunk_list, incomingPacket.payload[0]);
        sendGetSW(sock, from);
        free_chunks(chunk_list, incomingPacket.payload[0]);
        break;
    case GET:
        // sender
        dprintf(STDOUT_FILENO, "GET received\n");
        chunk_list = retrieve_chunk_list(&incomingPacket);
        jobs[nodeInMap] = getChunkId(chunk_list[0], config->has_chunk_file);
        // send a window to requester
        //congestion control started
        windowSize[nodeInMap] = 1;
        congestionState[nodeInMap] = 0;
        ssthresh[nodeInMap] = 64;
        lastSent[nodeInMap] = 0;
        for (i = 0; i < windowSize[nodeInMap]; i++)
            sendData(from, config);
        recordWindowSize(config);
        free_chunks(chunk_list, incomingPacket.payload[0]);
        break;
    case DATA:
        // requester
        dprintf(STDOUT_FILENO, "DATA received %d from %d\n",
                incomingPacket.sequence_num, nodeInMap);
        GETTTL[nodeInMap] = -1;

        // if sequence_num < expected, ignore as they might be channel-queued packets
        // if sequence_num > expected, something is lost or in wrong order
        if (incomingPacket.sequence_num > nextExpected[nodeInMap]
                && numMismatches < 3)
        {
            p = packet_factory(ACK);
            p->packet_length = 16;
            p->ack_num = nextExpected[nodeInMap] - 1;
            spiffy_sendto(sock, p, p->packet_length, 0,
                    (struct sockaddr *) &from, sizeof(from));
            numMismatches++;
            numDataMisses[nodeInMap] = 0;
        }
        else if (incomingPacket.sequence_num == nextExpected[nodeInMap])
        {
            numDataMisses[nodeInMap] = 0;
            numMismatches = 0;
            processData(&incomingPacket, config, sock, from);
            nextExpected[nodeInMap] = incomingPacket.sequence_num + 1;
            if (incomingPacket.sequence_num == BT_CHUNK_SIZE
                    && peers[nodeInMap]->next != NULL )
            {
                dprintf(STDOUT_FILENO, "Got %s\n", peers[nodeInMap]->chunkHash);
                linkNode *temp = peers[nodeInMap]; //cp1
                peers[nodeInMap] = peers[nodeInMap]->next;
                free(temp);
                sendGetSW(sock, from);
            }
            else if (incomingPacket.sequence_num == BT_CHUNK_SIZE
                    && peers[nodeInMap]->next == NULL )
            {
                dprintf(STDOUT_FILENO, "JOB is done\n");
                numDataMisses[nodeInMap] = -1;
            }
        }
        break;
    case ACK:
        dprintf(STDOUT_FILENO, "ACK received %d\n", incomingPacket.ack_num);
        // sender
        if (lastACKed[nodeInMap] == incomingPacket.ack_num)
            dup_ack[nodeInMap]++;
        // similarly, smaller received ack can be channel queued, ignore
        else if (incomingPacket.ack_num != lastACKed[nodeInMap] + 1)
            return;
        // duplicate acked
        else
            dup_ack[nodeInMap] = 0;

        // clear DATA TTL
        TTL[nodeInMap][incomingPacket.ack_num - 1] = -1;

        // retransmit
        if (dup_ack[nodeInMap] == 3) //loss
        {
            dprintf(STDOUT_FILENO, "dup retransmitting\n");
            congestionControlRetransmit(config); //change windowSize
            lastSent[nodeInMap] = lastACKed[nodeInMap];
            sendData(from, config);
            dup_ack[nodeInMap] = 0;
        }
        else //normal
        {
            congestionControlNormal(&incomingPacket, config);
            lastACKed[nodeInMap] = incomingPacket.ack_num;
            // ok... task finished
            if (incomingPacket.ack_num == BT_CHUNK_SIZE)
            {
                jobs[nodeInMap] = -1;
                lastACKed[nodeInMap] = 0;
                lastSent[nodeInMap] = 0;
                windowSize[nodeInMap] = 0;
                congestionState[nodeInMap] = -1;
                ssthresh[nodeInMap] = 64;
                resetCCRTT();
                dprintf(STDOUT_FILENO, "JOB is done\n");
                numDataMisses[nodeInMap] = -1;
            }
            else
            {
                for (i = lastSent[nodeInMap];
                        i < lastACKed[nodeInMap] + windowSize[nodeInMap]
                                && i < BT_CHUNK_SIZE; i++)
                    sendData(from, config);
                if (congestionState[nodeInMap] == CA) //test the RTT timer
                {
                    if (headRTT != NULL )
                    {
                        for (tempRtt = headRTT; tempRtt->next != NULL ;
                                tempRtt = tempRtt->next)
                            ;
                        tempRtt->next = (rttRecord *) malloc(sizeof(rttRecord));
                        tempRtt->next->seq = lastSent[nodeInMap];
                        tempRtt->next->next = NULL;
                    }
                    else
                    {
                        headRTT = (rttRecord *) malloc(sizeof(rttRecord));
                        headRTT->seq = lastSent[nodeInMap];
                        headRTT->next = NULL;
                    }
                }
            }
        }
        break;
    case DENIED:
        dprintf(STDOUT_FILENO, "DENIED received\n");
        break;
    default:
        perror("Unknown Command: Ignored...\n");
        break;
    }
}

char **process_get(char *chunkfile, char *outputfile)
{
    FILE *fp;
    FILE *outfp;
    char **chunk_list;
    char buf[USERBUF_SIZE];
    int i;

    i = 0;
    chunk_list = (char **) malloc(BT_MAX_NUM_CHUNKS * sizeof(char *));
    if (!(fp = fopen(chunkfile, "r")))
        return NULL ;

    while (fgets(buf, sizeof(buf), fp) != NULL )
    {
        chunk_list[i] = (char *) malloc(BT_CHUNKS_SIZE + 1);
        sscanf(buf, "%*d %s", chunk_list[i++]);
        memset(buf, 0, USERBUF_SIZE);
    }

    while (i < BT_MAX_NUM_CHUNKS)
        chunk_list[i++] = NULL;

    fclose(fp);

    if (!(outfp = fopen(outputfile, "wb")))
        return NULL ;
    fclose(outfp);

    return chunk_list;
}

void broadcast_query(char **chunk_list, bt_config_t *config)
{
    packet *p;
    int num_chunks, head;
    bt_peer_t *node;

    p = (packet *) malloc(sizeof(packet));

    dprintf(STDOUT_FILENO, "Sending packet payload has:\n");
    p = packet_factory(WHOHAS);
    for (num_chunks = 0; num_chunks < BT_MAX_NUM_CHUNKS; num_chunks++)
    {
        if (chunk_list[num_chunks] == NULL )
            break;
        head = 4 + num_chunks * BT_CHUNKS_SIZE;
        strncpy(p->payload + head, chunk_list[num_chunks], BT_CHUNKS_SIZE);
        dprintf(STDOUT_FILENO, "%s\n", chunk_list[num_chunks]);
    }

    p->packet_length = 20 + num_chunks * BT_CHUNKS_SIZE;
    p->payload[0] = num_chunks;
    p->payload[1] = '\0';
    p->payload[2] = '\0';
    p->payload[3] = '\0';

    for (node = config->peers; node != NULL ; node = node->next)
        if (node->id != config->identity)
            spiffy_sendto(config->sock, p, p->packet_length, 0,
                    (struct sockaddr *) &node->addr, sizeof(node->addr));
}

void handle_user_input(char *line, void *cbdata)
{
    char **chunk_list;
    bt_config_t *config;

    bzero(chunkf, sizeof(chunkf));
    bzero(outf, sizeof(outf));

    config = (bt_config_t *) cbdata;
    if (sscanf(line, "GET %120s %120s", chunkf, outf))
    {
        if (strlen(outf) > 0)
        {
            if ((chunk_list = process_get(chunkf, outf)) == NULL )
            {
                perror("I/O error");
                exit(-1);
            }

            // record all hash chunks for the current download
            work_queue = chunk_list;
            broadcast_query(chunk_list, config);
        }
    }
}

struct sockaddr_in *getAddr(bt_config_t *config, int i)
{
    bt_peer_t *curPeer;
    for (curPeer = config->peers; curPeer != NULL ; curPeer = curPeer->next)
        if (curPeer->id == i)
            return &curPeer->addr;
    return NULL ;
}

void peer_run(bt_config_t *config)
{
    struct sockaddr_in myaddr;
    fd_set readfds;
    struct user_iobuf *userbuf;
    struct timeval timeout;
    int i, j;

    if ((userbuf = create_userbuf()) == NULL )
    {
        perror("peer_run could not allocate userbuf");
        exit(-1);
    }

    if ((config->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1)
    {
        perror("peer_run could not create socket");
        exit(-1);
    }

    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY );
    myaddr.sin_port = htons(config->myport);

    if (bind(config->sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1)
    {
        perror("peer_run could not bind socket");
        exit(-1);
    }

    spiffy_init(config->identity, (struct sockaddr *) &myaddr, sizeof(myaddr));

    timeout.tv_sec = 1;
    while (1)
    {
        int nfds;
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(config->sock, &readfds);

        nfds = select(config->sock + 1, &readfds, NULL, NULL, &timeout);

        if (nfds > 0)
        {
            if (FD_ISSET(config->sock, &readfds))
                process_inbound_udp(config->sock, config);

            if (FD_ISSET(STDIN_FILENO, &readfds))
                process_user_input(STDIN_FILENO, userbuf, handle_user_input,
                        config);
        }
        // check timeout
        if (timeout.tv_sec == 0)
        {
            for (i = 0; i < BT_MAX_PEERS; i++)
            {
                for (j = 0; j < BT_CHUNK_SIZE; j++)
                {
                    if (TTL[i][j] != -1)
                        TTL[i][j]++;

                    // set the DATA timeout to 2
                    if (TTL[i][j] == 2)
                    {
                        dprintf(STDOUT_FILENO, "retransmitting on timeout %d\n",
                                j + 1);
                        lastSent[i] = j; // lastACKed should be the same as j
                        sendData(*getAddr(config, i), config);
                        dup_ack[i] = 0;
                        break;
                    }
                }
                if (GETTTL[i] != -1)
                    GETTTL[i]++;

                // set the GET timeout to 5
                if (GETTTL[i] == 5)
                {
                    if (++numGetMisses[i] > 3)
                    {
                        numGetMisses[i] = 0;
                        numDataMisses[i] = -1;
                        jobs[i] = -1;
                        dup_ack[i] = 0;
                        GETTTL[i] = -1;
                        windowSize[i] = 0;
                        congestionState[i] = -1;
                        if ((work_queue = process_get(chunkf, outf)) == NULL )
                        {
                            perror("I/O error");
                            exit(-1);
                        }
                        GETTTL[i] = -1;
                        broadcast_query(work_queue, config);
                    }
                    else
                    {
                        dprintf(STDOUT_FILENO, "resending GET\n");
                        if ((work_queue = process_get(chunkf, outf)) == NULL )
                        {
                            perror("I/O error");
                            exit(-1);
                        }
                        sendGetSW(config->sock, *getAddr(config, i));
                    }
                }

                if (numDataMisses[i] != -1)
                    numDataMisses[i]++;

                if (numDataMisses[i] > 15)
                {
                    numDataMisses[i] = -1;
                    jobs[i] = -1;
                    dup_ack[i] = 0;
                    GETTTL[i] = -1;
                    windowSize[i] = 0;
                    congestionState[i] = -1;
                    numGetMisses[i] = 0;
                    resetCCRTT();
                    if ((work_queue = process_get(chunkf, outf)) == NULL )
                    {
                        perror("I/O error");
                        exit(-1);
                    }
                    broadcast_query(work_queue, config);
                }
            }
            timeout.tv_sec = 1;
        }
    }
}
