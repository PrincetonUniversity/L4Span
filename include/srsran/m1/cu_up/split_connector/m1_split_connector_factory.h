#pragma once

#include "srsran/m1/cu_up/m1_gateway.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include <cstdint>

namespace srsran::srs_cu_up {

struct m1_cu_up_split_gateway_creation_msg {
  ngu_gateway& udp_gw;
  gtpu_demux&  demux;
  dlt_pcap&    gtpu_pcap;
  uint16_t     peer_port;
};

std::unique_ptr<srsran::m1_cu_up_udp_gateway> create_split_m1_gw(m1_cu_up_split_gateway_creation_msg msg);

} // namespace srsran::srs_cu_up