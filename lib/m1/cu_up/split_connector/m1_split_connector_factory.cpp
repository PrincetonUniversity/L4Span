#include "srsran/m1/cu_up/split_connector/m1_split_connector_factory.h"
#include "m1_split_connector.h"

using namespace srsran;
using namespace srs_cu_up;

std::unique_ptr<m1_cu_up_udp_gateway> srsran::srs_cu_up::create_split_m1_gw(m1_cu_up_split_gateway_creation_msg msg)
{
  return std::make_unique<m1_split_connector>(msg.udp_gw, msg.demux, msg.gtpu_pcap, msg.peer_port);
}