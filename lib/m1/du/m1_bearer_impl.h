#pragma once

#include "srsran/m1/du/m1_bearer.h"
#include "srsran/m1/du/m1_bearer_logger.h"
#include "srsran/m1/du/m1_config.h"
#include "srsran/m1/du/m1_rx_sdu_notifier.h"
#include "srsran/m1/du/m1_tx_pdu_notifier.h"
#include "srsran/ran/lcid.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/timers.h"

namespace srsran {
namespace srs_du {

class m1_bearer_impl final : public m1_bearer,
                             public m1_tx_sdu_handler,
                             public m1_rx_pdu_handler
{
public:
  m1_bearer_impl(uint32_t                       ue_index,
                 drb_id_t                       drb_id_,
                 const up_transport_layer_info& dl_tnl_info_,
                 const m1_config&               config,
                 m1_rx_sdu_notifier&            rx_sdu_notifier_,
                 m1_tx_pdu_notifier&            tx_pdu_notifier_,
                 timer_factory                  timers,
                 task_executor&                 ue_executor_);

  ~m1_bearer_impl() override { stop(); }

  m1_tx_sdu_handler&      get_tx_sdu_handler() override { return *this; }
  m1_rx_pdu_handler&      get_rx_pdu_handler() override { return *this; }

  void handle_sdu(mru_ul_buffer_state ul_message) override;

  void handle_pdu(mru_dl_message msg) override;

  void stop() override;
private:
  m1_bearer_logger  logger;
  bool              stopped = false;

  /// Config storage
  const m1_config               cfg;
  const up_transport_layer_info dl_tnl_info;

  m1_rx_sdu_notifier& rx_sdu_notifier;
  m1_tx_pdu_notifier& tx_pdu_notifier;

  task_executor& ue_executor;

  void handle_pdu_impl(mru_dl_message msg);

  // void on_expired_ul_notif_timer();
};

} // namespace srs_du
} // namespace srsran