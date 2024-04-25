#include "main.h"

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

void send_ping(int sockfd, struct sockaddr_in *dest_addr, int seq_num, struct options *opts) {
	//printf("----- %s -----\n", __FUNCTION__);
    struct icmp		icmp_packet;
    struct timeval	tv_out;
	char			packet[USHRT_MAX + sizeof(struct icmp)];

    memset(&icmp_packet, 0, sizeof(struct icmp));
    icmp_packet.icmp_type = ICMP_ECHO;
    icmp_packet.icmp_code = 0;
    icmp_packet.icmp_id = getpid();
    icmp_packet.icmp_seq = seq_num;
    icmp_packet.icmp_cksum = 0;

	memcpy(packet, &icmp_packet, sizeof(struct icmp));

    // Set packet size if specified
    if (opts->packet_size > 0 && opts->packet_size <= PACKET_SIZE)
	{
    	memset(((char *)packet) + sizeof(struct icmp), 'X', opts->packet_size - sizeof(struct icmp));
	}

	//printf("packet_size : %d\n", opts->packet_size);
	//printf("struct icmp size : %lu\n", sizeof(struct icmp));
	//printf("size of icmp packet : %lu\n", sizeof(icmp_packet));
	//printf("seq_num : %d\n", seq_num);
	//printf("total size : %u\n", opts->packet_size);
    
    icmp_packet.icmp_cksum = checksum(&icmp_packet, sizeof(struct icmp) + opts->packet_size - sizeof(struct icmp));

	int s = sendto(sockfd, &icmp_packet, sizeof(icmp_packet) + opts->packet_size - sizeof(struct icmp), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr));
	if (s <= 0) {
    		perror("sendto");
    		exit(EXIT_FAILURE);
	}
	//printf("Number of bytes sent : %d\n", s);


    gettimeofday(&tv_out, NULL);
}

void recv_ping(int sockfd, int seq_num, struct timeval *tv_in, struct options *opts, char domain_name[NI_MAXHOST]) {
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
		//printf("recv_size : %ld\n", recv_size);

        struct iphdr *ip_hdr = (struct iphdr *)recv_buf;
        struct icmp *icmp_hdr = (struct icmp *)(recv_buf + (ip_hdr->ihl << 2));
		//printf(
		//		"%d, %d | %d, %d | %d, %d\n", icmp_hdr->icmp_type, ICMP_ECHOREPLY, icmp_hdr->icmp_id, getpid(), icmp_hdr->icmp_seq, seq_num
		//	  );

        if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && icmp_hdr->icmp_id == getpid() && icmp_hdr->icmp_seq == seq_num) {
            gettimeofday(tv_in, NULL);
            double rtt = (tv_in->tv_sec - tv_out.tv_sec) * 1000.0 + (tv_in->tv_usec - tv_out.tv_usec) / 1000.0;
            printf("%ld bytes from %s (%s): icm_seq=%d ttl=%d time=%.1fms\n", recv_size - sizeof(struct iphdr), domain_name, inet_ntoa(recv_addr.sin_addr), seq_num + 1, ip_hdr->ttl, rtt);
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

    printf("PING %s (%s) %ld(%ld) bytes of data.\n", target, address, opts.packet_size - sizeof(struct icmphdr), opts.packet_size + sizeof(struct iphdr));

	while (1)
	{
		for (int i = 0; i < 4; i++) {
			struct timeval tv_in;
			send_ping(sockfd, &dest_addr, i, &opts);
			recv_ping(sockfd, i, &tv_in, &opts, domain_name);
			sleep(1);
		}
	}

    close(sockfd);
    return 0;
}
