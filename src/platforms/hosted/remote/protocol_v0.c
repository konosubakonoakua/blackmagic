/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2023 1BitSquared <info@1bitsquared.com>
 * Written by Rachel Mant <git@dragonmux.network>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "bmp_remote.h"
#include "swd.h"
#include "jtagtap.h"

#include "protocol_v0.h"
#include "protocol_v0_defs.h"
#include "protocol_v0_swd.h"
#include "protocol_v0_jtag.h"
#include "protocol_v0_adiv5.h"

static bool remote_v0_adiv5_init(adiv5_debug_port_s *dp);
static bool remote_v0_plus_adiv5_init(adiv5_debug_port_s *dp);

void remote_v0_init(void)
{
	DEBUG_WARN("Probe firmware does not support the newer JTAG commands or ADIv5 acceleration, please update it.\n");
	remote_funcs = (bmp_remote_protocol_s){
		.swd_init = remote_v0_swd_init,
		.jtag_init = remote_v0_jtag_init,
		.adiv5_init = remote_v0_adiv5_init,
	};
}

void remote_v0_plus_init(void)
{
	DEBUG_WARN("Probe firmware does not support the newer JTAG commands or ADIv5 acceleration, please update it.\n");
	remote_funcs = (bmp_remote_protocol_s){
		.swd_init = remote_v0_swd_init,
		.jtag_init = remote_v0_jtag_init,
		.adiv5_init = remote_v0_plus_adiv5_init,
	};
}

bool remote_v0_swd_init(void)
{
	DEBUG_PROBE("remote_swd_init\n");
	platform_buffer_write(REMOTE_SWD_INIT_STR, sizeof(REMOTE_SWD_INIT_STR));

	char buffer[REMOTE_MAX_MSG_SIZE];
	const int length = platform_buffer_read(buffer, REMOTE_MAX_MSG_SIZE);
	if (!length || buffer[0] == REMOTE_RESP_ERR) {
		DEBUG_ERROR("remote_swd_init failed, error %s\n", length ? buffer + 1 : "unknown");
		return false;
	}

	swd_proc.seq_in = remote_v0_swd_seq_in;
	swd_proc.seq_in_parity = remote_v0_swd_seq_in_parity;
	swd_proc.seq_out = remote_v0_swd_seq_out;
	swd_proc.seq_out_parity = remote_v0_swd_seq_out_parity;
	return true;
}

bool remote_v0_jtag_init(void)
{
	DEBUG_PROBE("remote_jtag_init\n");
	platform_buffer_write(REMOTE_JTAG_INIT_STR, sizeof(REMOTE_JTAG_INIT_STR));

	char buffer[REMOTE_MAX_MSG_SIZE];
	const int length = platform_buffer_read(buffer, REMOTE_MAX_MSG_SIZE);
	if (!length || buffer[0] == REMOTE_RESP_ERR) {
		DEBUG_ERROR("remote_jtag_init failed, error %s\n", length ? buffer + 1 : "unknown");
		return false;
	}

	jtag_proc.jtagtap_reset = remote_v0_jtag_reset;
	jtag_proc.jtagtap_next = remote_v0_jtag_next;
	jtag_proc.jtagtap_tms_seq = remote_v0_jtag_tms_seq;
	jtag_proc.jtagtap_tdi_tdo_seq = remote_v0_jtag_tdi_tdo_seq;
	jtag_proc.jtagtap_tdi_seq = remote_v0_jtag_tdi_seq;
	jtag_proc.tap_idle_cycles = 1;
	return true;
}

static bool remote_v0_adiv5_init(adiv5_debug_port_s *const dp)
{
	(void)dp;
	DEBUG_WARN("Falling back to non-accelerated probe interface\n");
	DEBUG_WARN("Please update your probe's firmware for a substantial speed increase\n");
	return true;
}

static bool remote_v0_plus_adiv5_init(adiv5_debug_port_s *const dp)
{
	dp->low_access = remote_v0_adiv5_raw_access;
	dp->dp_read = remote_v0_adiv5_dp_read;
	dp->ap_read = remote_v0_adiv5_ap_read;
	dp->ap_write = remote_v0_adiv5_ap_write;
	dp->mem_read = remote_v0_adiv5_mem_read_bytes;
	dp->mem_write = remote_v0_adiv5_mem_write_bytes;
	return true;
}
