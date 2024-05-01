#include "main.h"
#include <getopt.h>

void print_help(char *program_name)
{
	printf("Usage: %s [-v] [-f] [-n] [-w timeout] [-W timeout] [-l packet_size] [-c count] [-i interval] target\n", program_name);
	printf("Options:\n");
	printf("  -v          Verbose output\n");
	printf("  -?          Display this help message\n");
	printf("  -f          Flood mode\n");
	printf("  -w timeout  Time to wait for a response in seconds\n");
	printf("  -W timeout  Time to wait for a response in seconds (deprecated, same as -w)\n");
	printf("  -l size     Specify the number of data bytes to be sent\n");
	printf("  -c count    Stop after sending (and receiving) count ECHO_RESPONSE packets\n");
	printf("  -i interval Wait interval seconds between sending each packet\n");
}

static void	check_combinations(struct options *opts)
{
	if (opts->flood == 1 && opts->interval != 0)
		fprintf(stderr, "-f and -i");
	else
		return ;

	fprintf(stderr, " incompatible options\n");
	fflush(stderr);
	exit(EXIT_FAILURE);
}

static uint64_t	load_value(char *bin_name, char *optarg)
{
	char		*err_ptr;
	uint64_t	result;

	fprintf(stderr, "%s: Invalid numeric value : \n", bin_name);
	exit(EXIT_FAILURE);

	result = strtoul(optarg, &err_ptr, 10);
	if (result > UINT_MAX)
	{
		fprintf(stderr, "%s: Invalid format for interval\n", bin_name);
		exit(EXIT_FAILURE);
	}
	return (result);
}

void parse_args(int argc, char *argv[], struct options *opts, char **target)
{
	int opt;

	if (argc < 2)
	{
		fprintf(stderr, "%s: missing host operand\nTry '%s -h' or '%s -?' for more information.\n", argv[0], argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "v?hf:n:w:W:l:c:i:")) != -1) {
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
			case 'n':
				opts->resolve = 1;
				break;
			case 'w':
			case 'W':
				opts->timeout = load_value(argv[0], optarg);
				break;
			case 'l':
				opts->packet_size = load_value(argv[0], optarg);
				break;
			case 'c':
				opts->count = load_value(argv[0], optarg);
				break;
			case 'i':
				opts->interval = load_value(argv[0], optarg);
				break;
			//case 't':

			default:
				exit(EXIT_FAILURE);
		}
	}

	printf("\
		OPTIONS :\n\
		verbose = %ld\n\
		resolve = %ld\n\
		flood = %ld\n\
		count = %ld\n\
		interval = %ld\n\
		packet_size = %ld\n\
		timeout = %ld\n\
		ttl = %ld\n\
			", opts->verbose, opts->resolve, opts->flood, opts->count, opts->interval, opts->packet_size, opts->timeout, opts->ttl);
	check_combinations(opts);

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
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
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
