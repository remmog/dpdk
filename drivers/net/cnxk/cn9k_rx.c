/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(C) 2021 Marvell.
 */

#include "cn9k_ethdev.h"
#include "cn9k_rx.h"

#define R(name, f3, f2, f1, f0, flags)					       \
	uint16_t __rte_noinline __rte_hot cn9k_nix_recv_pkts_##name(	       \
		void *rx_queue, struct rte_mbuf **rx_pkts, uint16_t pkts)      \
	{                                                                      \
		return cn9k_nix_recv_pkts(rx_queue, rx_pkts, pkts, (flags));   \
	}

NIX_RX_FASTPATH_MODES
#undef R

static inline void
pick_rx_func(struct rte_eth_dev *eth_dev,
	     const eth_rx_burst_t rx_burst[2][2][2][2])
{
	struct cnxk_eth_dev *dev = cnxk_eth_pmd_priv(eth_dev);

	/* [MARK] [CKSUM] [PTYPE] [RSS] */
	eth_dev->rx_pkt_burst = rx_burst
		[!!(dev->rx_offload_flags & NIX_RX_OFFLOAD_MARK_UPDATE_F)]
		[!!(dev->rx_offload_flags & NIX_RX_OFFLOAD_CHECKSUM_F)]
		[!!(dev->rx_offload_flags & NIX_RX_OFFLOAD_PTYPE_F)]
		[!!(dev->rx_offload_flags & NIX_RX_OFFLOAD_RSS_F)];
}

void
cn9k_eth_set_rx_function(struct rte_eth_dev *eth_dev)
{
	const eth_rx_burst_t nix_eth_rx_burst[2][2][2][2] = {
#define R(name, f3, f2, f1, f0, flags)					\
	[f3][f2][f1][f0] = cn9k_nix_recv_pkts_##name,

		NIX_RX_FASTPATH_MODES
#undef R
	};

	pick_rx_func(eth_dev, nix_eth_rx_burst);

	rte_mb();
}