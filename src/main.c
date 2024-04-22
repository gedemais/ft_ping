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

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

void print_help(char *program_name) {
    printf("Usage: %s [-v] [-f] [-n] [-w timeout] [-W timeout] [-l packet_size] [-c count] [-i interval] target\n", program_name);
    printf("Options:\n");
    printf("  -v          Verbose output\n");
    printf("  -?          Display this help message\n");
    printf("  -f          Flood mode\n");
    printf("  -n          Numeric output only\n");
    printf("  -w timeout  Time to wait for a response in seconds\n");
    printf("  -W timeout  Time to wait for a response in seconds (deprecated, same as -w)\n");
    printf("  -l size     Specify the number of data bytes to be sent\n");
    printf("  -c count    Stop after sending (and receiving) count ECHO_RESPONSE packets\n");
    printf("  -i interval Wait interval seconds between sending each packet\n");
}

void parse_args(int argc, char *argv[], struct options *opts, char **target) {
    int opt;

    while ((opt = getopt(argc, argv, "v?fnw:W:l:c:i:")) != -1) {
        switch (opt) {
            case 'v':
                opts->verbose = 1;
                break;
            case 'f':
                opts->flood = 1;
                break;
            case 'n':
                // Not implemented
                break;
            case 'w':
            case 'W':
                opts->timeout = atoi(optarg);
                break;
            case 'l':
                opts->packet_size = atoi(optarg);
                break;
            case 'c':
                opts->count = atoi(optarg);
                break;
            case 'i':
                opts->interval = atoi(optarg);
                break;
            case '?':
            default:
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
        }
    }

    if (optind < argc)
        *target = argv[optind];
    else {
        fprintf(stderr, "Target host not specified.\n");
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_num, struct options *opts) {
    struct icmp icmp_packet;
    struct timeval tv_out;

    memset(&icmp_packet, 0, sizeof(struct icmp));
    icmp_packet.icmp_type = ICMP_ECHO;
    icmp_packet.icmp_code = 0;
    icmp_packet.icmp_id = getpid();
    icmp_packet.icmp_seq = seq_num;
    icmp_packet.icmp_cksum = 0;
    
    // Set packet size if specified
    if (opts->packet_size > 0 && opts->packet_size <= PACKET_SIZE)
        memset(((char *)&icmp_packet) + sizeof(struct icmp), 'X', opts->packet_size - sizeof(struct icmp));

    icmp_packet.icmp_cksum = checksum(&icmp_packet, sizeof(struct icmp) + opts->packet_size - sizeof(struct icmp));

    if (sendto(sockfd, &icmp_packet, sizeof(icmp_packet) + opts->packet_size - sizeof(struct icmp), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) <= 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&tv_out, NULL);
}

void recv_ping(int sockfd, int seq_num, struct timeval *tv_in, struct options *opts) {
    struct timeval tv_out;
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    char recv_buf[MAX_PACKET_SIZE];
    ssize_t recv_size;

    gettimeofday(&tv_out, NULL);

    while (1) {
        struct timeval timeout;
        timeout.tv_sec = opts->timeout;
        timeout.tv_usec = 0;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (ready == 0) {
            printf("Request timed out for sequence %d\n", seq_num);
            return;
        }

        recv_size = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
        if (recv_size <= 0) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }

        struct iphdr *ip_hdr = (struct iphdr *)recv_buf;
        struct icmp *icmp_hdr = (struct icmp *)(recv_buf + (ip_hdr->ihl << 2));

        if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && icmp_hdr->icmp_id == getpid() && icmp_hdr->icmp_seq == seq_num) {
            gettimeofday(tv_in, NULL);
            double rtt = (tv_in->tv_sec - tv_out.tv_sec) * 1000.0 + (tv_in->tv_usec - tv_out.tv_usec) / 1000.0;
            printf("Received reply from %s: seq=%d time=%.2fms\n", inet_ntoa(recv_addr.sin_addr), seq_num, rtt);
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    struct options opts = {0};
    char *target = NULL;

    parse_args(argc, argv, &opts, &target);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, target, &dest_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    printf("PING %s\n", target);

    for (int i = 0; i < 4; i++) {
        struct timeval tv_in;
        send_ping(sockfd, &dest_addr, i, &opts);
        recv_ping(sockfd, i, &tv_in, &opts);
        sleep(1);
    }

    close(sockfd);
    return 0;
}

