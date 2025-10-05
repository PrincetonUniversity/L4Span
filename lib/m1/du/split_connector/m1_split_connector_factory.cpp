#include "srsran/m1/du/split_connector/m1_split_connector_factory.h"
#include "m1_split_connector.h"

using namespace srsran;
using namespace srs_du;

std::unique_ptr<m1_du_udp_gateway> srsran::srs_du::create_split_m1_gw(m1_du_split_gateway_creation_msg msg)
{
  return std::make_unique<m1_split_connector>(msg.udp_gw, msg.demux, msg.gtpu_pcap, msg.peer_port, msg.m1_ext_addr);
}