#pragma once 

#include "srsran/mru/mru_message.h"

struct sockaddr_storage;

namespace srsran {

/****************************************
 * Interfaces/notifiers for the gateway
 ****************************************/
/// This interface represents the data exit point of the receiving side of a GTP-U NR-U (F1-U) entity.
/// The GTP-U will use this notifier to pass GTP-U SDUs (i.e MR-U DL/UL messages) into the lower layer.
class gtpu_tunnel_mru_rx_lower_layer_notifier
{
public:
  gtpu_tunnel_mru_rx_lower_layer_notifier()                                                           = default;
  virtual ~gtpu_tunnel_mru_rx_lower_layer_notifier()                                                  = default;
  gtpu_tunnel_mru_rx_lower_layer_notifier(const gtpu_tunnel_mru_rx_lower_layer_notifier&)             = delete;
  gtpu_tunnel_mru_rx_lower_layer_notifier& operator=(const gtpu_tunnel_mru_rx_lower_layer_notifier&)  = delete;
  gtpu_tunnel_mru_rx_lower_layer_notifier(const gtpu_tunnel_mru_rx_lower_layer_notifier&&)            = delete;
  gtpu_tunnel_mru_rx_lower_layer_notifier& operator=(const gtpu_tunnel_mru_rx_lower_layer_notifier&&) = delete;

  /// \brief Interface for the GTP-U to pass a SDU (i.e. NR-U DL message) into the lower layer, right now it does nothing.
  /// \param dl_message NR-U DL message with optional T-PDU.
  virtual void on_new_sdu(mru_dl_message dl_message) = 0;

  /// \brief Interface for the GTP-U to pass a SDU (i.e. NR-U UL message) into the lower layer.
  /// \param ul_message NR-U UL message with optional T-PDU.
  virtual void on_new_sdu(mru_ul_buffer_state ul_message) = 0;
};

} // namespace srsran