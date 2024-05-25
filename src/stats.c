#include "main.h"

void	display_stats(struct stats *stats)
{
	uint64_t	lost_packets;

	lost_packets = stats->sent_packets - stats->received_packets;
	if (stats->sent_packets > 0)
		stats->packet_loss = ((float)lost_packets / (float)stats->sent_packets) * 100;
	else
		stats->packet_loss = 0;

	printf("--- %s ping statistics ---\n", stats->target);
	printf("%u packets transmitted, %u packets received, %u%% packet loss\n", stats->sent_packets, stats->received_packets, stats->packet_loss);
}
