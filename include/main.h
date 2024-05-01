#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <signal.h>
#include <ctype.h>

#define PACKET_SIZE 64
#define MAX_PACKET_SIZE 65536
#define TIMEOUT_SEC 1

struct	stats
{
	uint32_t	sent_packets;
	uint32_t	received_packets;
	uint32_t	packet_loss;
	uint32_t	time;
};

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
void	parse_args(int argc, char *argv[], struct options *opts, char **target);
void	print_help(char *program_name);
int		get_hostname_address(char *hostname, char address[INET_ADDRSTRLEN]);
int		get_domain_name(char ip_addr[INET_ADDRSTRLEN], char domain_name[NI_MAXHOST]);
void	send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_num, struct options *opts);
