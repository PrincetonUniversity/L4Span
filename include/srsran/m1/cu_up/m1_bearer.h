#pragma once

#include "m1_rx_pdu_handler.h"
#include "m1_tx_sdu_handler.h"
#include "srsran/ran/up_transport_layer_info.h"

namespace srsran {
namespace srs_cu_up {

constexpr uint32_t m1_dl_notif_time_ms = 10;

class m1_bearer_disconnector;

class m1_bearer
{
public:
  m1_bearer()          = default;
  virtual ~m1_bearer() = default;

  m1_bearer(const m1_bearer&)            = delete;
  m1_bearer(m1_bearer&&)                 = delete;
  m1_bearer& operator=(const m1_bearer&) = delete;
  m1_bearer& operator=(m1_bearer&&)      = delete;

  virtual void                stop()               = 0;
  virtual m1_rx_pdu_handler& get_rx_pdu_handler() = 0;
  virtual m1_tx_sdu_handler& get_tx_sdu_handler() = 0;
};

/// This class represents the interface through which a M1 bearer disconnects from its gateway upon destruction
class m1_bearer_disconnector
{
public:
  virtual ~m1_bearer_disconnector() = default;

  virtual void disconnect_cu_bearer(const up_transport_layer_info& ul_up_tnl_info) = 0;
};

} // namespace srs_cu_up
} // namespace srsran