#pragma once 

#include "m1_bearer.h"
#include "m1_rx_sdu_notifier.h"
#include "m1_tx_pdu_notifier.h"
#include "srsran/m1/cu_up/m1_config.h"
#include "srsran/ran/lcid.h"
#include "srsran/support/timers.h"

namespace srsran {
namespace srs_cu_up {

/// \brief Creates an M1 bearer for the CU-UP
std::unique_ptr<m1_bearer> create_m1_bearer(uint32_t                       ue_index,
                                            drb_id_t                       drb_id,
                                            const up_transport_layer_info& ul_up_tnl_info,
                                            const m1_config&               config,
                                            m1_tx_pdu_notifier&            tx_pdu_notifier,
                                            m1_rx_sdu_notifier&            rx_sdu_notifier,
                                            timer_factory                  ue_dl_timer_factory,
                                            unique_timer&                  ue_inactivitity_timer,
                                            task_executor&                 ul_exec);

} // namespace srs_cu_up
} // namespace srsran