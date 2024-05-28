#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include <float.h>

#define PACKET_SIZE 64
#define MAX_PACKET_SIZE 65536
#define TIMEOUT_SEC 1

struct	stats
{
	uint32_t	sent_packets;
	uint32_t	received_packets;
	uint32_t	packet_loss;
	char		*target;
	uint32_t	pad;
};

struct options {
    uint64_t	verbose;
    uint64_t	resolve;
	uint64_t	packet_size;
    uint64_t	flood;
    uint64_t	pad;
    uint64_t	count;
    float		interval;
    float		timeout;
    uint64_t	ttl;
};

// Utilities
void			parse_args(int argc, char *argv[], struct options *opts, char **target);
void			print_help(char *program_name);
int				get_hostname_address(char *hostname, char address[INET_ADDRSTRLEN]);
int				get_domain_name(char ip_addr[INET_ADDRSTRLEN], char domain_name[NI_MAXHOST]);
void			display_stats(struct stats *stats);
void			send_ping(int sockfd, struct sockaddr_in *dest_addr, uint64_t seq_num, struct options *opts, struct stats *stats);
struct stats	*stats_singleton(void);
void			print_ip_data(struct ip *ip);
