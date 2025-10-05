#include "srsran/gtpu/gtpu_tunnel_mru_factory.h"
#include "gtpu_tunnel_impl_mru.h"

using namespace srsran;

std::unique_ptr<gtpu_tunnel_mru> srsran::create_gtpu_tunnel_mru(gtpu_tunnel_mru_creation_message& msg)
{
  return std::make_unique<gtpu_tunnel_mru_impl>(msg.ue_index, msg.cfg, *msg.gtpu_pcap, *msg.rx_lower, *msg.tx_upper);
}

std::unique_ptr<gtpu_tunnel_common_rx_upper_layer_interface>
srsran::create_gtpu_tunnel_mru_rx(gtpu_tunnel_mru_rx_creation_message& msg)
{
  return std::make_unique<gtpu_tunnel_mru_rx_impl>(msg.ue_index, msg.rx_cfg, *msg.rx_lower);
}

std::unique_ptr<gtpu_tunnel_mru_tx_lower_layer_interface>
srsran::create_gtpu_tunnel_mru_tx(gtpu_tunnel_mru_tx_creation_message& msg)
{
  return std::make_unique<gtpu_tunnel_mru_tx_impl>(msg.ue_index, msg.tx_cfg, *msg.gtpu_pcap, *msg.tx_upper);
}
