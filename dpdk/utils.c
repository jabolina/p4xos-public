#include <signal.h>
#include <rte_ip.h>
#include <rte_log.h>
#include "utils.h"
#include "const.h"

/**
 * Parse an ethernet header to fill the ethertype, outer_l2_len, outer_l3_len and
 * ipproto. This function is able to recognize IPv4/IPv6 with one optional vlan
 * header.
 */
void
parse_ethernet(struct ether_hdr *eth_hdr, union tunnel_offload_info *info,
		uint8_t *l4_proto)
{
	struct ipv4_hdr *ipv4_hdr;
	struct ipv6_hdr *ipv6_hdr;
	uint16_t ethertype;

	info->outer_l2_len = sizeof(struct ether_hdr);
	ethertype = rte_be_to_cpu_16(eth_hdr->ether_type);

	if (ethertype == ETHER_TYPE_VLAN) {
		struct vlan_hdr *vlan_hdr = (struct vlan_hdr *)(eth_hdr + 1);
		info->outer_l2_len  += sizeof(struct vlan_hdr);
		ethertype = rte_be_to_cpu_16(vlan_hdr->eth_proto);
	}

	switch (ethertype) {
	case ETHER_TYPE_IPv4:
		ipv4_hdr = (struct ipv4_hdr *)
			((char *)eth_hdr + info->outer_l2_len);
		info->outer_l3_len = sizeof(struct ipv4_hdr);
		*l4_proto = ipv4_hdr->next_proto_id;
		break;
	case ETHER_TYPE_IPv6:
		ipv6_hdr = (struct ipv6_hdr *)
			((char *)eth_hdr + info->outer_l2_len);
		info->outer_l3_len = sizeof(struct ipv6_hdr);
		*l4_proto = ipv6_hdr->proto;
		break;
	default:
		info->outer_l3_len = 0;
		*l4_proto = 0;
		break;
	}
}

void
print_paxos_hdr(struct paxos_hdr *p)
{
	rte_log(RTE_LOG_INFO, RTE_LOGTYPE_USER8,
		"{ .msgtype=%u, .inst=%u, .rnd=%u, .vrnd=%u, .acptid=%u\n",
		rte_be_to_cpu_16(p->msgtype),
		rte_be_to_cpu_32(p->inst),
		rte_be_to_cpu_16(p->rnd),
		rte_be_to_cpu_16(p->vrnd),
		rte_be_to_cpu_16(p->acptid));
}

uint16_t
get_psd_sum(void *l3_hdr, uint16_t ethertype, uint64_t ol_flags)
{
	if (ethertype == ETHER_TYPE_IPv4)
		return rte_ipv4_phdr_cksum(l3_hdr, ol_flags);
	else /* assume ethertype == ETHER_TYPE_IPv6 */
		return rte_ipv6_phdr_cksum(l3_hdr, ol_flags);
}

void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		rte_log(RTE_LOG_DEBUG, RTE_LOGTYPE_USER8,
				"\n\nSignal %d received, preparing to exit...\n", signum);
		force_quit = true;
	}
}


