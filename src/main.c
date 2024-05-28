#include "main.h"

void recv_ping(int sockfd, int seq_num, struct timeval *tv_in, struct options *opts)
{
	//printf("----- %s -----\n", __FUNCTION__);
	struct timeval tv_out;
	struct sockaddr_in recv_addr;
	struct stats	*stats;
	socklen_t addr_len = sizeof(recv_addr);
	char recv_buf[MAX_PACKET_SIZE];
	ssize_t recv_size;

	stats = stats_singleton();
	gettimeofday(&tv_out, NULL);

	while (1) {
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
		if (ready < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (ready == 0) {
			return;
		}

		recv_size = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
		if (recv_size <= 0) {
			perror("recvfrom");
			exit(EXIT_FAILURE);
		}

		struct iphdr	*ip_hdr = (struct iphdr *)recv_buf;
		struct icmp		*icmp_hdr = (struct icmp *)(recv_buf + (ip_hdr->ihl << 2));
		struct ip		*ip = &icmp_hdr->icmp_ip;

		if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && icmp_hdr->icmp_id == getpid() && icmp_hdr->icmp_seq == seq_num)
		{
			gettimeofday(tv_in, NULL);
			double rtt = (tv_in->tv_sec - tv_out.tv_sec) * 1000.0 + (tv_in->tv_usec - tv_out.tv_usec) / 1000.0;
			printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr), seq_num, ip_hdr->ttl, rtt);
			stats->received_packets++;
			fflush(stdout);
			return;
		}
		else
		{
			if (icmp_hdr->icmp_type == ICMP_TIME_EXCEEDED && icmp_hdr->icmp_code == ICMP_EXC_TTL)
				printf("%ld bytes from %s: Time to Live exceeded (TTL expired)\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr));
			else if (icmp_hdr->icmp_type == ICMP_DEST_UNREACH) {
				switch (icmp_hdr->icmp_code) {
					case ICMP_NET_UNREACH:
						printf("%ld bytes from %s: Destination Net Unreachable\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr));
						break;
					case ICMP_HOST_UNREACH:
						printf("%ld bytes from %s: Destination Host Unreachable\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr));
						break;
					case ICMP_PROT_UNREACH:
						printf("%ld bytes from %s: Destination Protocol Unreachable\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr));
						break;
					case ICMP_PORT_UNREACH:
						printf("%ld bytes from %s: Destination Port Unreachable\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr));
						break;
					default:
						printf("%ld bytes from %s: Destination Unreachable, code=%d\n", recv_size - sizeof(struct iphdr), inet_ntoa(recv_addr.sin_addr), icmp_hdr->icmp_code);
						break;
				}
			}

			if (opts->verbose)
				print_ip_data(ip);
			fflush(stdout);
		}
	}
}

void handle_sigint(int sig) {
	(void)sig;
	struct	stats *stats;

	stats = stats_singleton();
	display_stats(stats);
	exit(0);
}

int	open_raw_socket(void)
{
	int	sockfd;
	struct protoent *proto = getprotobyname ("icmp");

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		if (errno == EPERM || errno == EACCES)
		{
			errno = 0;

			sockfd = socket (AF_INET, SOCK_DGRAM, proto->p_proto);
			if (sockfd < 0)
			{
				if (errno == EPERM || errno == EACCES || errno == EPROTONOSUPPORT)
					fprintf (stderr, "ping: Lacking privilege for icmp socket.\n");
				else
					fprintf (stderr, "ping: %s\n", strerror(errno));

				return (-1);
			}
		}
		else
			return (-1);
	}
	return (sockfd);
}


int main(int argc, char *argv[]) {
	struct		options opts = {0};
	struct		stats *stats;
	char		*target = NULL;
	char		address[INET_ADDRSTRLEN];
	char		domain_name[NI_MAXHOST];
	const int	pid = getpid();

	static struct timeval exec_start, now, uptime;
	gettimeofday(&exec_start, NULL);

	memset(&opts, 0, sizeof(struct options));
	opts.packet_size = 64; // Default packet size
	opts.timeout = 1000; // Default timeout
	opts.interval = FLT_MAX; // Default interval
	parse_args(argc, argv, &opts, &target);

	int sockfd = open_raw_socket();
	if (sockfd < 0) {
		exit(EXIT_FAILURE);
	}

	int one = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof(one));
	setsockopt (sockfd, IPPROTO_IP, IP_TTL, &opts.ttl, sizeof(opts.ttl));

	/* Reset root privileges */
	if (setuid (getuid ()) != 0)
	{
		perror(strerror(errno));
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
		exit(EXIT_FAILURE);
	}

	printf("PING %s (%s): %ld data bytes", target, address, opts.packet_size - sizeof(struct icmphdr));
	if (opts.verbose == 1)
		printf(", id 0x%04x = %d", pid, pid);
	printf ("\n");

	signal(SIGINT, handle_sigint);

	stats = stats_singleton();
	stats->target = target;
	uint64_t i = 0;

	// Ping main loop
	while (opts.count == 0 || i < opts.count)
	{
		struct timeval tv_in; 
		send_ping(sockfd, &dest_addr, i, &opts, stats);
		recv_ping(sockfd, i, &tv_in, &opts);
		i++;
		i == opts.count ? (void)i : usleep(opts.interval * 1000000.0f);

		gettimeofday(&now, NULL);
		timersub(&now, &exec_start, &uptime);

		if (opts.timeout > 0 && uptime.tv_sec > opts.timeout)
		{
			display_stats(stats);
			close(sockfd);
			return (0);
		}
	}

	close(sockfd);

	display_stats(stats);
	return (0);
}
