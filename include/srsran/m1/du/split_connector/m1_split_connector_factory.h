#pragma once

#include "srsran/m1/du/m1_gateway.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include <cstdint>

namespace srsran::srs_du {

struct m1_du_split_gateway_creation_msg {
  srs_cu_up::ngu_gateway* udp_gw;
  gtpu_demux*             demux;
  dlt_pcap&               gtpu_pcap;
  uint16_t                peer_port;
  std::string             m1_ext_addr = "auto";
};

std::unique_ptr<m1_du_udp_gateway> create_split_m1_gw(m1_du_split_gateway_creation_msg msg);

} // namespace srsran::srs_du
