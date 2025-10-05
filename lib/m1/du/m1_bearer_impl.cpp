#include "m1_bearer_impl.h"

using namespace srsran;
using namespace srs_du;

m1_bearer_impl::m1_bearer_impl(uint32_t                       ue_index,
                               drb_id_t                       drb_id_,
                               const up_transport_layer_info& dl_tnl_info_,
                               const m1_config&               config,
                               m1_rx_sdu_notifier&            rx_sdu_notifier_,
                               m1_tx_pdu_notifier&            tx_pdu_notifier_,
                               timer_factory                  timers,
                               task_executor&                 ue_executor_) :
  logger("DU-M1", {ue_index, drb_id_, dl_tnl_info_}),
  cfg(config),
  dl_tnl_info(dl_tnl_info_),
  rx_sdu_notifier(rx_sdu_notifier_),
  tx_pdu_notifier(tx_pdu_notifier_),
  ue_executor(ue_executor_)
  {

    logger.log_info("M1 bearer configured. {} {}", cfg, dl_tnl_info);
  }

void m1_bearer_impl::handle_sdu(mru_ul_buffer_state ul_message) 
{
  logger.log_debug("M1 bearer received UL message with t_pdu size={}", ul_message.t_pdu.length());

  tx_pdu_notifier.on_new_pdu(std::move(ul_message));
}

void m1_bearer_impl::handle_pdu(mru_dl_message msg)
{
  auto fn = [this, m = std::move(msg)] () mutable {handle_pdu_impl(std::move(m)) ;};
  if (!ue_executor.execute(std::move(fn))) {
    if (!cfg.warn_on_drop) {
      logger.log_info("Dropped M1 PDU, queue is full");
    } else {
      logger.log_warning("Dropped M1 PDU, queue is full");
    }
  }
}

void m1_bearer_impl::stop() {
  stopped = true;
}

void m1_bearer_impl::handle_pdu_impl(mru_dl_message msg)
{
  logger.log_debug("M1 bearer received PDU");
  // handle T-PDU
  if (!msg.t_pdu.empty()) {
    logger.log_debug("Delivering T-PDU. size={}", msg.t_pdu.length());
    rx_sdu_notifier.on_new_sdu(std::move(msg.t_pdu));
  }
  // handle discard notifications
  // if (msg.dl_user_data.discard_blocks.has_value()) {
  //   nru_pdcp_sn_discard_blocks& blocks     = msg.dl_user_data.discard_blocks.value();
  //   size_t                      nof_blocks = blocks.size();
  //   for (size_t block_idx = 0; block_idx < nof_blocks; block_idx++) {
  //     const nru_pdcp_sn_discard_block& block       = blocks[block_idx];
  //     uint32_t                         pdcp_sn_end = block.pdcp_sn_start + block.block_size;
  //     for (uint32_t pdcp_sn = block.pdcp_sn_start; pdcp_sn < pdcp_sn_end; pdcp_sn++) {
  //       logger.log_debug("Notifying PDU discard for pdcp_sn={}", pdcp_sn);
  //       rx_sdu_notifier.on_discard_sdu(pdcp_sn);
  //     }
  //   }
  // }
}