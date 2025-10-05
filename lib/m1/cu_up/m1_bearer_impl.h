#pragma once

#include "srsran/m1/cu_up/m1_bearer.h"
#include "srsran/m1/cu_up/m1_bearer_logger.h"
#include "srsran/m1/cu_up/m1_config.h"
#include "srsran/m1/cu_up/m1_rx_sdu_notifier.h"
#include "srsran/m1/cu_up/m1_tx_pdu_notifier.h"
#include "srsran/ran/lcid.h"
#include "srsran/support/timers.h"

namespace srsran {
namespace srs_cu_up {

class m1_bearer_impl final : public m1_bearer, public m1_rx_pdu_handler, public m1_tx_sdu_handler
{
public:
  m1_bearer_impl(uint32_t                       ue_index,
                 drb_id_t                       drb_id_,
                 const up_transport_layer_info& ul_tnl_info_,
                 const m1_config&               config,
                 m1_tx_pdu_notifier&            tx_pdu_notifier_,
                 m1_rx_sdu_notifier&            rx_sdu_notifier_,
                 timer_factory                  ue_dl_timer_factory,
                 unique_timer&                  ue_inactivity_timer_,
                 task_executor&                 ul_exec_);
  m1_bearer_impl(const m1_bearer_impl&)            = delete;
  m1_bearer_impl& operator=(const m1_bearer_impl&) = delete;
  
  ~m1_bearer_impl() override { stop(); }

  m1_rx_pdu_handler& get_rx_pdu_handler() override { return *this; }
  m1_tx_sdu_handler& get_tx_sdu_handler() override { return *this; }

  void stop() override { dl_notif_timer.stop(); }

  void handle_pdu(mru_ul_buffer_state msg) override;
  void handle_sdu(byte_buffer sdu) override;

  void discard_sdu(uint32_t pdcp_sn) override;

  /// \brief Returns the UL tunnel info that was assigned upon construction.
  const up_transport_layer_info& get_ul_tnl_info() const { return ul_tnl_info; }

  /// \brief Returns the UL TEID that was assigned upon construction.
  /// \return The UL TEID associated with this bearer.
  gtpu_teid_t get_ul_teid() const { return ul_tnl_info.gtp_teid; }

private:
  m1_bearer_logger logger;

  /// Config storage
  const m1_config cfg;

  m1_tx_pdu_notifier&      tx_pdu_notifier;
  m1_rx_sdu_notifier&      rx_sdu_notifier;
  up_transport_layer_info  ul_tnl_info;
  task_executor&           ul_exec;

  /// Sentinel value representing a not-yet set PDCP SN
  static constexpr uint32_t unset_pdcp_sn = UINT32_MAX;

  /// Downlink notification timer that triggers periodic transmission of discard blocks towards lower layers. The
  /// purpose of this timer is to avoid excessive downlink notifications for every PDCP SN that is discarded by upper
  /// layers.
  unique_timer dl_notif_timer;

  /// UE inactivity timer that is injected from parent entities. The timer must run in the UL executor!
  /// The timer shall be restarted on each UL PDU (= UL activity) and on each transmit notification (= DL activity).
  unique_timer& ue_inactivity_timer;

  /// Holds the last highest transmitted PDCP SN that was reported to upper layers
  uint32_t notif_highest_transmitted_pdcp_sn = unset_pdcp_sn;
  /// Holds the last highest delivered PDCP SN that was reported to upper layers
  uint32_t notif_highest_delivered_pdcp_sn = unset_pdcp_sn;

  /// Collection of pending \c nru_pdcp_sn_discard_block objects.
  nru_pdcp_sn_discard_blocks discard_blocks;

  /// @brief  Uplink feedback from the RLC layer to the MARK layer.
  /// @param msg the actual msg, but the actual data's structure should be defined.
  void handle_pdu_impl(mru_ul_buffer_state msg);

  // /// \brief Fills the provided \c nru_dl_message with all SDU discard blocks that have been aggregated since the last
  // /// call of this function (or since creation of this object).
  // /// \param msg The message to be filled with SDU discard blocks.
  // void fill_discard_blocks(nru_dl_message& msg);

  // /// \brief Immediately transmits a \c nru_dl_message with all currently aggregated SDU discard blocks, if any.
  // void flush_discard_blocks();
};

} // namespace srs_cu_up
} // namespace srsran