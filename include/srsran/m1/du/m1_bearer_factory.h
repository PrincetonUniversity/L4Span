#pragma once

#include "m1_bearer.h"
#include "m1_rx_sdu_notifier.h"
#include "m1_tx_pdu_notifier.h"
#include "srsran/m1/du/m1_config.h"
#include "srsran/ran/lcid.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/timers.h"

namespace srsran {
namespace srs_du {

struct m1_bearer_creation_message {
  uint32_t                 ue_index;
  drb_id_t                 drb_id;
  up_transport_layer_info  dl_tnl_info;
  m1_config                config;
  m1_rx_sdu_notifier*      rx_sdu_notifier;
  m1_tx_pdu_notifier*      tx_pdu_notifier;
  timer_factory            timers;
  task_executor*           ue_executor;
  m1_bearer_disconnector*  disconnector;
};

/// \brief Creates an F1-U bearer for the DU.
std::unique_ptr<m1_bearer> create_m1_bearer(const m1_bearer_creation_message& msg);

} // namespace srs_du
} // namespace srsran
