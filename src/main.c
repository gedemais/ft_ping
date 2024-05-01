#include "main.h"

void recv_ping(int sockfd, int seq_num, struct timeval *tv_in, struct options *opts) {
	//printf("----- %s -----\n", __FUNCTION__);
    struct timeval tv_out;
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    char recv_buf[MAX_PACKET_SIZE];
    ssize_t recv_size;

    gettimeofday(&tv_out, NULL);

    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = opts->timeout * 1000;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (ready == 0) {
            //printf("Request timed out for sequence %d\n", seq_num);
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
            printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr), seq_num, ip_hdr->ttl, rtt);
			fflush(stdout);
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    struct options opts = {0};
    char	*target = NULL;
	char	address[INET_ADDRSTRLEN];
	char	domain_name[NI_MAXHOST];

    opts.packet_size = 64; // Default packet size
	opts.timeout = 1000;
    parse_args(argc, argv, &opts, &target);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

	if (get_hostname_address(target, address) < 0)
		return (1);

	if (get_domain_name(address, domain_name) < 0)
		return (1);

    int s = inet_pton(AF_INET, address, &dest_addr.sin_addr);
    if (s <= 0) {
        if (s == 0)
            fprintf(stderr, "Not in presentation format\n");
        else
            perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    printf("PING %s (%s): %ld data bytes.\n", target, address, opts.packet_size - sizeof(struct icmphdr));

	while (1)
	{
		for (int i = 0; i < INT_MAX; i++) {
			struct timeval tv_in;
			send_ping(sockfd, &dest_addr, i, &opts);
			recv_ping(sockfd, i, &tv_in, &opts);
		}
	}

    close(sockfd);
    return 0;
}
