#include "srsran/m1/du/m1_bearer_factory.h"
#include "m1_bearer_impl.h"

using namespace srsran;
using namespace srs_du;

std::unique_ptr<m1_bearer> srsran::srs_du::create_m1_bearer(const m1_bearer_creation_message& msg)
{
  srsran_assert(msg.rx_sdu_notifier != nullptr, "Cannot create M1 bearer: RX SDU notifier is not configured.");
  srsran_assert(msg.tx_pdu_notifier != nullptr, "Cannot create M1 bearer: TX PDU notifier is not configured.");
  srsran_assert(msg.ue_executor != nullptr, "Cannot create M1 bearer: UE executor is not configured.");
  srsran_assert(msg.disconnector != nullptr, "Cannot create M1 bearer: disconnector is not configured.");
  auto bearer = std::make_unique<m1_bearer_impl>(msg.ue_index,
                                                 msg.drb_id,
                                                 msg.dl_tnl_info,
                                                 msg.config,
                                                 *msg.rx_sdu_notifier,
                                                 *msg.tx_pdu_notifier,
                                                 msg.timers,
                                                 *msg.ue_executor);
  return bearer;
}