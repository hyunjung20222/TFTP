#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

#define ERROR -1
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERR 5

// buffer includes packet (exclude data packet)
// BUFSIZ alreday defined in header
static char buf[BUFSIZ];

static char err_msg[4][40] = { "Not defined Error",
                        "File not Found",
                        "Unknown Transfer ID (TID)",
                        "File already exists"
                        };

// create packet function (RRQ, WRQ, ACK, ERR)
int req_packet(int opcode, char * filename, char *mode, char buf[]);
int ack_packet(short int block, char buf[]);
int err_packet(int err_code, char *err_msg, char buf[]);

// print usage function
void usage(void);