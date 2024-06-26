#include "main.h"
#include <getopt.h>

static void	check_combinations(struct options *opts, char *bin_name)
{
	if (opts->flood == 1 && opts->interval != FLT_MAX)
		fprintf(stderr, "%s: -f and -i incompatible options\n", bin_name);
	else
		return ;

	exit(EXIT_FAILURE);
}

static uint64_t	load_int_value(char *bin_name, char *optarg)
{
	char		*err_ptr;
	uint64_t	result;


	result = strtoul(optarg, &err_ptr, 10);
	if (result > UINT_MAX)
	{
		fprintf(stderr, "%s: Invalid number format : %s\n", bin_name, err_ptr);
		exit(EXIT_FAILURE);
	}
	return (result);
}

static float	load_float_value(char *bin_name, char *optarg)
{
	char		*err_ptr;
	float		result;

	result = strtof(optarg, &err_ptr);
	if (result > UINT_MAX)
	{
		fprintf(stderr, "%s: Invalid number format : %s\n", bin_name, err_ptr);
		exit(EXIT_FAILURE);
	}
	return (result);
}


void print_help(char *prog_name) {
	printf("Usage: %s [OPTION...] HOST ...\n", prog_name);
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("Options:\n");
	printf("  -v, --verbose       Enable verbose output\n");
	printf("  -h, --help          Display this help message\n");
	printf("  -f  --flood         Enable flood mode\n");
	printf("  -w, --timeout       Set timeout value\n");
	printf("  -c  --count         Set packets count\n");
	printf("  -i  --interval      Set interval\n");
	printf("  -t  --ttl           Set time to live\n");
	exit(EXIT_SUCCESS);
}

void parse_args(int argc, char *argv[], struct options *opts, char **target) {
	int opt;
	const struct option long_options[] = {
		{"verbose", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, 'h'},
		{"flood", no_argument, NULL, 'f'},
		{"timeout", required_argument, NULL, 'w'},
		{"count", required_argument, NULL, 'c'},
		{"interval", required_argument, NULL, 'i'},
		{"ttl", required_argument, NULL, 't'},
		{NULL, 0, NULL, 0}
	};

	if (argc < 2) {
		fprintf(stderr, "%s: missing host operand\nTry '%s -h' or '%s --help' for more information.\n", argv[0], argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt_long(argc, argv, "v?hfw:c:i:t:", long_options, NULL)) != -1) {
		switch (opt) {
			case 'v':
				opts->verbose = 1;
				break;
			case '?':
			case 'h':
				print_help(argv[0]);
				break;
			case 'f':
				opts->flood = 1;
				break;
			case 'w':
				opts->timeout = load_float_value(argv[0], optarg);
				break;
			case 'c':
				opts->count = load_int_value(argv[0], optarg);
				break;
			case 'i':
				opts->interval = load_float_value(argv[0], optarg);
				break;
			case 't':
				opts->ttl = load_int_value(argv[0], optarg);
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}

	check_combinations(opts, argv[0]);

	if (opts->interval == FLT_MAX)
		opts->interval = opts->flood == 1 ? 0 : 1;

	/*printf("\
	OPTIONS :\n\
	verbose = %ld\n\
	resolve = %ld\n\
	flood = %ld\n\
	count = %ld\n\
	interval = %f\n\
	timeout = %f\n\
	ttl = %ld\n\
	", opts->verbose, opts->resolve, opts->flood, opts->count, opts->interval, opts->timeout, opts->ttl);*/

	if (optind < argc)
		*target = argv[optind];
	else {
		fprintf(stderr, "%s: missing host operand\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

int	get_hostname_address(char *hostname, char address[INET_ADDRSTRLEN])
{
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(hostname, NULL, &hints, &res);
	if (status != 0) {
		fprintf(stderr, "./ft_ping: unknown host\n");
		return 1;
	}

	struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &(ipv4->sin_addr), address, INET_ADDRSTRLEN);

	freeaddrinfo(res);
	return (0);
}

int get_domain_name(char ip_addr[INET_ADDRSTRLEN], char domain_name[NI_MAXHOST])
{
	struct sockaddr_in sa;
	// Initialize the sockaddr_in structure
	sa.sin_family = AF_INET;
	inet_pton(AF_INET, ip_addr, &(sa.sin_addr));

	// Perform the reverse DNS lookup
	int result = getnameinfo((struct sockaddr *)&sa, sizeof(sa), domain_name, NI_MAXHOST, NULL, 0, 0);
	if (result != 0) {
		fprintf(stderr, "Error: %s\n", gai_strerror(result));
		return -1; // Return an error code if the lookup fails
	}

	return 0; // Return 0 if successful
}

struct stats *stats_singleton(void)
{
	static struct stats stats;
	static bool			first = true;

	if (first)
	{
		bzero(&stats, sizeof(struct stats));
		first = false;
	}

	return (&stats);
}

void	print_ip_data(struct ip *ip)
{
	size_t hlen = ip->ip_hl << 2;
	unsigned char *cp = (unsigned char *) ip + sizeof (*ip);

	printf ("IP Hdr Dump:\n ");
	for (size_t j = 0; j < sizeof (*ip); ++j)
		printf ("%02x%s", *((unsigned char *) ip + j), (j % 2) ? " " : "");	/* Group bytes two by two.  */

	printf ("\n");
	printf
	("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
	printf (" %1x  %1x  %02x", ip->ip_v, ip->ip_hl, ip->ip_tos);
	/*
	 * The member `ip_len' is not portably reported in any byte order.
	 * Use a simple heuristic to print a reasonable value.
	 */
	printf (" %04x %04x",
			(ip->ip_len > 0x2000) ? ntohs (ip->ip_len) : ip->ip_len,
			ntohs (ip->ip_id));
	printf ("   %1x %04x", (ntohs (ip->ip_off) & 0xe000) >> 13,
			ntohs (ip->ip_off) & 0x1fff);
	printf ("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ntohs (ip->ip_sum));
	printf (" %s ", inet_ntoa (*((struct in_addr *) &ip->ip_src)));
	printf (" %s ", inet_ntoa (*((struct in_addr *) &ip->ip_dst)));
	while (hlen-- > sizeof (*ip))
		printf ("%02x", *cp++);

    int type = *cp;
    int code = *(cp + 1);
	printf ("\n");
	printf ("ICMP: type %u, code %u, size %zu", type, code, ntohs(ip->ip_len) - hlen);
	printf (", id 0x%04x, seq 0x%04x", *(cp + 4) * 256 + *(cp + 5), *(cp + 6) * 256 + *(cp + 7));
	printf ("\n");
}
