#include "main.h"

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
