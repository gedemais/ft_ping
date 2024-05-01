#include "main.h"

static unsigned short checksum(void *b, int len) {
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
