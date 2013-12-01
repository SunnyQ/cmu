/*
 * bt_parse.h
 *
 * Initial Author: Ed Bardsley <ebardsle+441@andrew.cmu.edu>
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 command line and config file parsing
 * stubs.
 *
 */

#ifndef _BT_PARSE_H_
#define _BT_PARSE_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BT_FILENAME_LEN 255
#define BT_MAX_PEERS 1000
#define BT_MAX_NUM_CHUNKS 4096
#define BT_CHUNKS_SIZE 40 // or 20??? TODO: confirm!!
#define BT_CHUNK_SIZE 512
#define BT_FULL_DATASIZE 1024

#define WHOHAS 0
#define IHAVE 1
#define GET 2
#define DATA 3
#define ACK 4
#define DENIED 5

#define INIT -1
#define SLOWSTART 0
#define CA 1

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct ln
{
    char chunkHash[BT_CHUNKS_SIZE + 1];
    struct ln *next;
} linkNode;

typedef struct rRD
{
	int seq;
	struct rRD *next;
} rttRecord;

typedef struct bt_peer_s
{
    short id;
    struct sockaddr_in addr;
    struct bt_peer_s *next;
} bt_peer_t;

typedef struct
{
    short magic_num;
    char version;
    char type;
    short header_length;
    short packet_length;
    int sequence_num;
    int ack_num;
    char payload[1484]; // total packet size should be 1500
} packet;

struct bt_config_s
{
    char chunk_file[BT_FILENAME_LEN];
    char has_chunk_file[BT_FILENAME_LEN];
    char output_file[BT_FILENAME_LEN];
    char peer_list_file[BT_FILENAME_LEN];
    int max_conn;
    short identity;
    unsigned short myport;
    int sock;

    int argc;
    char **argv;

    bt_peer_t *peers;
};
typedef struct bt_config_s bt_config_t;

void bt_init(bt_config_t *c, int argc, char **argv);
void bt_parse_command_line(bt_config_t *c);
void bt_parse_peer_list(bt_config_t *c);
void bt_dump_config(bt_config_t *c);
bt_peer_t *bt_peer_info(const bt_config_t *c, int peer_id);

#endif /* _BT_PARSE_H_ */
