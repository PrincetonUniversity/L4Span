#pragma once 

#include "gtpu_tunnel_mru_rx_impl.h"
#include "gtpu_tunnel_mru_tx_impl.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_tunnel_mru.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/srslog/logger.h"

namespace srsran {

class gtpu_tunnel_mru_impl : public gtpu_tunnel_mru
{
public:
  gtpu_tunnel_mru_impl(srs_cu_up::ue_index_t                       ue_index,
                       gtpu_tunnel_mru_config                      cfg,
                       dlt_pcap&                                   gtpu_pcap,
                       gtpu_tunnel_mru_rx_lower_layer_notifier&    rx_lower,
                       gtpu_tunnel_common_tx_upper_layer_notifier& tx_upper) :
    logger(srslog::fetch_basic_logger("GTPU"))
  {
    rx = std::make_unique<gtpu_tunnel_mru_rx_impl>(ue_index, cfg.rx, rx_lower);
    tx = std::make_unique<gtpu_tunnel_mru_tx_impl>(ue_index, cfg.tx, gtpu_pcap, tx_upper);
  }
  ~gtpu_tunnel_mru_impl() override = default;

  gtpu_tunnel_common_rx_upper_layer_interface* get_rx_upper_layer_interface() final { return rx.get(); }
  gtpu_tunnel_mru_tx_lower_layer_interface*    get_tx_lower_layer_interface() final { return tx.get(); }
private:
  srslog::basic_logger& logger;
  
  std::unique_ptr<gtpu_tunnel_mru_rx_impl> rx = {};
  std::unique_ptr<gtpu_tunnel_mru_tx_impl> tx = {};
};

} // namespace srsran