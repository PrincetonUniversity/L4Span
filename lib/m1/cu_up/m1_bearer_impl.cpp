#include "m1_bearer_impl.h"
#include <sstream>
#include <iomanip>

using namespace srsran;
using namespace srs_cu_up;

m1_bearer_impl::m1_bearer_impl(uint32_t                       ue_index,
                               drb_id_t                       drb_id_,
                               const up_transport_layer_info& ul_tnl_info_,
                               const m1_config&               config,
                               m1_tx_pdu_notifier&            tx_pdu_notifier_,
                               m1_rx_sdu_notifier&            rx_sdu_notifier_,
                               timer_factory                  ue_dl_timer_factory,
                               unique_timer&                  ue_inactivity_timer_,
                               task_executor&                 ul_exec_) :
  logger("CU-M1", {ue_index, drb_id_, ul_tnl_info_}),
  cfg(config),
  tx_pdu_notifier(tx_pdu_notifier_),
  rx_sdu_notifier(rx_sdu_notifier_),
  ul_tnl_info(ul_tnl_info_),
  ul_exec(ul_exec_),
  dl_notif_timer(ue_dl_timer_factory.create_timer()),
  ue_inactivity_timer(ue_inactivity_timer_)
{}

void m1_bearer_impl::handle_pdu(mru_ul_buffer_state msg)
{
  auto fn = [this, m = std::move(msg)] () mutable { handle_pdu_impl(std::move(m)); };

  if (not ul_exec.defer(std::move(fn))) {
    if (!cfg.warn_on_drop) {
      logger.log_info("Dropped M1 PDU, but why?");
    } else {
      logger.log_warning("Dropped M1 PDU, but why?");
    }
  }
}

void m1_bearer_impl::handle_sdu(byte_buffer sdu)
{
  logger.log_debug("M1 bearer received SDU. size={}", sdu.length());
  mru_dl_message msg = {};

  // attach the SDU
  msg.t_pdu = std::move(sdu);

  tx_pdu_notifier.on_new_pdu(std::move(msg));
}

void m1_bearer_impl::discard_sdu(uint32_t pdcp_sn)
{
  logger.log_debug("M1 bearer discard? To be removed.");
}

void m1_bearer_impl::handle_pdu_impl(mru_ul_buffer_state msg)
{
  logger.log_debug("M1 bearer received PDU, {} bytes.", msg.t_pdu.length());
  /// TODO: Pass the msg to the mark entity for packet marking.
  rx_sdu_notifier.on_new_sdu(std::move(msg.t_pdu), msg.drb_id);
}

// void m1_bearer_impl::fill_discard_blocks(nru_dl_message& msg)
// {
//   if (!discard_blocks.empty()) {
//     msg.dl_user_data.discard_blocks = std::move(discard_blocks);
//     discard_blocks                  = {};
//   }
// }

// void m1_bearer_impl::on_expired_dl_notif_timer()
// {
//   logger.log_debug("DL notification timer expired");
//   flush_discard_blocks();
// }

// void m1_bearer_impl::flush_discard_blocks()
// {
//   nru_dl_message msg = {};

//   // attach discard blocks (if any)
//   fill_discard_blocks(msg);

//   // stop backoff timer
//   dl_notif_timer.stop();

//   if (msg.dl_user_data.discard_blocks.has_value()) {
//     logger.log_debug("Sending discard blocks, to be removed.");
//     tx_pdu_notifier.on_new_pdu(std::move(msg));
//   }
// }
