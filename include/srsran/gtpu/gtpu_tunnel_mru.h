#pragma once

#include "srsran/gtpu/gtpu_tunnel_common_rx.h"
#include "srsran/gtpu/gtpu_tunnel_mru_tx.h"

namespace srsran {

/// Class used to interface with an GTP-U tunnel.
/// It will contain getters for the TX and RX entities interfaces.
/// TX and RX is considered from the perspective of the GTP-U.
class gtpu_tunnel_mru
{
public:
  gtpu_tunnel_mru()                                   = default;
  virtual ~gtpu_tunnel_mru()                          = default;
  gtpu_tunnel_mru(const gtpu_tunnel_mru&)             = delete;
  gtpu_tunnel_mru& operator=(const gtpu_tunnel_mru&)  = delete;
  gtpu_tunnel_mru(const gtpu_tunnel_mru&&)            = delete;
  gtpu_tunnel_mru& operator=(const gtpu_tunnel_mru&&) = delete;

  virtual gtpu_tunnel_common_rx_upper_layer_interface* get_rx_upper_layer_interface() = 0;
  virtual gtpu_tunnel_mru_tx_lower_layer_interface*    get_tx_lower_layer_interface() = 0;
};

} // namespace srsran