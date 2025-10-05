#include "srsran/m1/cu_up/m1_bearer_factory.h"
#include "m1_bearer_impl.h"

using namespace srsran;
using namespace srs_cu_up;

std::unique_ptr<m1_bearer> srsran::srs_cu_up::create_m1_bearer(uint32_t                       ue_index,
                                                               drb_id_t                       drb_id,
                                                               const up_transport_layer_info& ul_up_tnl_info,
                                                               const m1_config&               config,
                                                               m1_tx_pdu_notifier&            tx_pdu_notifier,
                                                               m1_rx_sdu_notifier&            rx_sdu_notifier,
                                                               timer_factory                  ue_dl_timer_factory,
                                                               unique_timer&                  ue_inactivitity_timer,
                                                               task_executor&                 ul_exec)
{
  auto bearer = std::make_unique<m1_bearer_impl>(ue_index,
                                                 drb_id,
                                                 ul_up_tnl_info,
                                                 config,
                                                 tx_pdu_notifier,
                                                 rx_sdu_notifier,
                                                 ue_dl_timer_factory,
                                                 ue_inactivitity_timer,
                                                 ul_exec);
  return bearer;
}