#pragma once

#include "srsran/cu_up/cu_up_types.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_tunnel_common_tx.h"
#include "srsran/gtpu/gtpu_tunnel_mru.h"
#include "srsran/gtpu/gtpu_tunnel_mru_rx.h"
#include "srsran/pcap/dlt_pcap.h"
#include <memory>

/// This factory header file depends on the GTP-U tunnel interfaces (see above include list). It is kept separate as
/// clients of the GTP-U tunnel interfaces do not need to call factory methods.
namespace srsran {

struct gtpu_tunnel_mru_creation_message {
  srs_cu_up::ue_index_t                       ue_index;
  gtpu_tunnel_mru_config                      cfg;
  dlt_pcap*                                   gtpu_pcap;
  gtpu_tunnel_mru_rx_lower_layer_notifier*    rx_lower;
  gtpu_tunnel_common_tx_upper_layer_notifier* tx_upper;
};

/// Creates an instance of a GTP-U entity at DU.
std::unique_ptr<gtpu_tunnel_mru> create_gtpu_tunnel_mru(gtpu_tunnel_mru_creation_message& msg);

struct gtpu_tunnel_mru_rx_creation_message {
  srs_cu_up::ue_index_t                             ue_index;
  gtpu_tunnel_mru_config::gtpu_tunnel_mru_rx_config rx_cfg;
  gtpu_tunnel_mru_rx_lower_layer_notifier*          rx_lower;
};

/// Create an instance of a GTP-U Rx entity at CU-UP.
std::unique_ptr<gtpu_tunnel_common_rx_upper_layer_interface>
create_gtpu_tunnel_mru_rx(gtpu_tunnel_mru_rx_creation_message& msg);

struct gtpu_tunnel_mru_tx_creation_message {
  srs_cu_up::ue_index_t                             ue_index;
  gtpu_tunnel_mru_config::gtpu_tunnel_mru_tx_config tx_cfg;
  dlt_pcap*                                         gtpu_pcap;
  gtpu_tunnel_common_tx_upper_layer_notifier*       tx_upper;
};

/// Create an instance of a GTP-U Tx entity at CU-UP.
std::unique_ptr<gtpu_tunnel_mru_tx_lower_layer_interface>
create_gtpu_tunnel_mru_tx(gtpu_tunnel_mru_tx_creation_message& msg);

} // namespace srsran
