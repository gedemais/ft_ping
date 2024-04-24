#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 64
#define MAX_PACKET_SIZE 65536
#define TIMEOUT_SEC 1

struct options {
    int verbose;
    int flood;
    int count;
    int interval;
    int packet_size;
    int timeout;
    int ttl;
};

// Utilities
void parse_args(int argc, char *argv[], struct options *opts, char **target);
void print_help(char *program_name);
