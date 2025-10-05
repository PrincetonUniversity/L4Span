#pragma once

#include "m1_rx_pdu_handler.h"
#include "m1_tx_sdu_handler.h"
#include "srsran/ran/up_transport_layer_info.h"

namespace srsran {
namespace srs_du {

class m1_bearer
{
public:
  virtual ~m1_bearer() = default;

  virtual m1_rx_pdu_handler&      get_rx_pdu_handler()      = 0;
  virtual m1_tx_sdu_handler&      get_tx_sdu_handler()      = 0;

  virtual void stop() = 0;
};

/// This class represents the interface through which a F1-U bearer disconnects from its gateway upon removal
class m1_bearer_disconnector
{
public:
  virtual ~m1_bearer_disconnector() = default;

  virtual void remove_du_bearer(const up_transport_layer_info& dl_up_tnl_info) = 0;
};

} // namespace srs_du
} // namespace srsran
