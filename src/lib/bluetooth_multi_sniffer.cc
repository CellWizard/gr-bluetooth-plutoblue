/* -*- c++ -*- */
/*
 * Copyright 2008, 2009 Dominic Spill, Michael Ossmann
 * Copyright 2007 Dominic Spill
 * Copyright 2005, 2006 Free Software Foundation, Inc.
 * 
 * This file is part of gr-bluetooth
 * 
 * gr-bluetooth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * gr-bluetooth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with gr-bluetooth; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bluetooth_multi_sniffer.h"
#include "bluetooth_packet.h"

/*
 * Create a new instance of bluetooth_multi_sniffer and return
 * a boost shared_ptr.  This is effectively the public constructor.
 */
bluetooth_multi_sniffer_sptr
bluetooth_make_multi_sniffer(double sample_rate, double center_freq,
		double squelch_threshold, bool tun)
{
	return bluetooth_multi_sniffer_sptr (new
		bluetooth_multi_sniffer(sample_rate, center_freq,
		squelch_threshold, tun));
}

/* private constructor */
bluetooth_multi_sniffer::bluetooth_multi_sniffer(double sample_rate,
		double center_freq, double squelch_threshold, bool tun)
	: bluetooth_multi_block(sample_rate, center_freq, squelch_threshold)
{
	d_tun = tun;
	set_symbol_history(3125);
	printf("lowest channel: %d, highest channel %d\n",
			d_low_channel, d_high_channel);

	/* Tun interface */
	if (d_tun) {
		d_chan_name = "gr-bluetooth";
		if ((d_tunfd = mktun(d_chan_name, d_ether_addr)) == -1) {
			fprintf(stderr,
				"warning: was not able to open TUN device, "
				"disabling Wireshark interface\n");
			// throw std::runtime_error("cannot open TUN device");
		}
	}

}

/* destructor */
bluetooth_multi_sniffer::~bluetooth_multi_sniffer ()
{
}

int 
bluetooth_multi_sniffer::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	int i;   /* index to start of packet */
	int ch;  /* channel number (0-78) */
	int len; /* number of symbols available */
	char symbols[history()];

	for (ch = d_low_channel; ch <= d_high_channel; ch++) {
		len = channel_symbols(ch, input_items, symbols,
				history() + noutput_items);

		/* skip this time slot if we didn't break squelch */
		if (len < 68)
			break;

		/* pointer to our starting place for sniff_ac */
		char *symp = symbols;

		/* don't look beyond one slot for ACs */
		int limit = (len - 68) < 625 ? (len - 68) : 625;

		/* look for multiple packets in this slot */
		while (limit >= 0 ) {
			i = bluetooth_packet::sniff_ac(symp, limit);
			if (i > -1) {
				ac(symp + i, len - i, ch);
				len -= (i + 68);
				symp += (i + 68);
				limit -= (i + 68);
			} else {
				break;
			}
		}
	}
	d_cumulative_count += (int) d_samples_per_slot;

	/* 
	 * The runtime system wants to know how many output items we produced,
	 * assuming that this is equal to the number of input items consumed.
	 * We tell it that we produced/consumed one time slot of input items so
	 * that our next run starts one slot later.
	 */
	return (int) d_samples_per_slot;
}

/* handle AC */
void bluetooth_multi_sniffer::ac(char *symbols, int len, int channel)
{
	uint32_t lap;
	uint32_t clkn;  /* native (local) clock in 625 us */
	bluetooth_piconet_sptr pn;
	
	clkn = (int) (d_cumulative_count / d_samples_per_slot) & 0x7ffffff;

	bluetooth_packet_sptr pkt = bluetooth_make_packet(symbols, len, clkn,
			channel);
	lap = pkt->get_LAP();

	printf("time %6d, channel %2d, LAP %06x ", clkn, channel, lap);

	//dump_hdr(symbols);
	if (pkt->header_present()) {
		if (!d_piconets[lap])
			d_piconets[lap] = bluetooth_make_piconet(lap);
		pn = d_piconets[lap];

		if (pn->have_clk6() && pn->have_UAP()) {
			decode(pkt, pn, true);
		} else {
			discover(pkt, pn);
		}
	} else {
		id(lap);
	}
}

/* handle ID packet (no header) */
void bluetooth_multi_sniffer::id(uint32_t lap)
{
	printf("ID\n");
	if (d_tun)
		write_interface(d_tunfd, NULL, 0, 0, lap, ETHER_TYPE);
}

/* decode packets with headers */
void bluetooth_multi_sniffer::decode(bluetooth_packet_sptr pkt,
		bluetooth_piconet_sptr pn, bool first_run)
{
	int clock; /* CLK of target piconet */

	clock = (pkt->d_clkn + pn->get_offset()) & 0x3f;
	pkt->set_clock(clock);
	pkt->set_UAP(pn->get_UAP());
	//printf("clock 0x%02x: ", clock);
	pkt->decode();

	if (pkt->got_payload()) {
		pkt->print();
		if (d_tun) {
			/* include 3 bytes for packet header */
			int length = pkt->get_payload_length() + 3;
			char *data = pkt->tun_format();
			int addr = (pkt->get_UAP() << 24) | pkt->get_LAP();
			write_interface(d_tunfd, (unsigned char *)data, length,
					0, addr, ETHER_TYPE);
			free(data);
		}
		if (pkt->get_type() == 2)
			fhs(pkt);
	} else if (first_run) {
		printf("lost clock!\n");
		pn->reset();

		/* start rediscovery with this packet */
		discover(pkt, pn);
	} else {
		printf("Giving up on queued packet!\n");
	}
}

/* work on UAP/CLK1-6 discovery */
void bluetooth_multi_sniffer::discover(bluetooth_packet_sptr pkt,
		bluetooth_piconet_sptr pn)
{
	printf("working on UAP/CLK1-6\n");

	/* store packet for decoding after discovery is complete */
	pn->enqueue(pkt);

	if (pn->UAP_from_header(pkt))
		/* success! decode the stored packets */
		recall(pn);
}

/* decode stored packets */
void bluetooth_multi_sniffer::recall(bluetooth_piconet_sptr pn)
{
	bluetooth_packet_sptr pkt;
	int i;
	printf("Decoding queued packets\n");

	while (pkt = pn->dequeue()) {
		printf("time %6d, channel %2d, LAP %06x ", pkt->d_clkn,
				pkt->d_channel, pkt->get_LAP());
		decode(pkt, pn, false);
	}

	printf("Finished decoding queued packets\n");
}

/* pull information out of FHS packet */
void bluetooth_multi_sniffer::fhs(bluetooth_packet_sptr pkt)
{
	uint32_t lap;
	uint8_t uap;
	uint32_t clk;
	uint32_t offset;
	bluetooth_piconet_sptr pn;

	/* caller should have checked got_payload() and get_type() */

	lap = pkt->lap_from_fhs();
	uap = pkt->uap_from_fhs();

	/* clk is shifted to put it into units of 625 microseconds */
	clk = pkt->clock_from_fhs() << 1;
	offset = (clk - pkt->d_clkn) & 0x7ffffff;

	printf("FHS contents: LAP %06x, UAP %02x, CLK %07x\n", lap, uap, clk);

	/* make use of this information from now on */
	if (!d_piconets[lap])
		d_piconets[lap] = bluetooth_make_piconet(lap);
	pn = d_piconets[lap];
	
	pn->set_UAP(uap);
	pn->set_offset(offset);
	//FIXME if this is a role switch, the offset can have an error of as
	//much as 1.25 ms 
}

/*
void bluetooth_multi_sniffer::dump_hdr(char *symbols)
{
	int i;
	for (i = 0; i < 72; i++)
		printf("%d", symbols[i]);
	printf("\n");
	for (; i < 126; i++) {
		printf("%d", symbols[i]);
		if (i % 3 == 2)
			printf(" ");
	}
	printf("\n");
}
*/
