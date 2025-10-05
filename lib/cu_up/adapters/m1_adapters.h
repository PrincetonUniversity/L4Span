#pragma once

#include "srsran/m1/cu_up/m1_gateway.h"
#include "srsran/m1/cu_up/m1_rx_sdu_notifier.h"
#include "srsran/mark/mark_m1_rx.h"
#include "srsran/mark/mark_m1_tx.h"
#include "srsran/srslog/srslog.h"

namespace srsran {
namespace srs_cu_up {

/// Adapter between M1 and PDCP
class m1_mark_adapter final : public m1_rx_sdu_notifier
{
public:
  void connect_mark(mark_rx_m1_interface& mark_rx_handler_, mark_tx_m1_interface& mark_tx_handler_)
  {
    mark_rx_handler = &mark_rx_handler_;
    mark_tx_handler = &mark_tx_handler_;
  }

  void on_new_sdu(byte_buffer sdu, drb_id_t drb_id) override
  {
    if (mark_rx_handler == nullptr) {
      srslog::fetch_basic_logger("M1").warning("Unconnected MARK handler. Dropping M1 SDU");
    } else {
      mark_rx_handler->handle_feedback(std::move(sdu), drb_id);
    }
  }

  // void on_transmit_notification(uint32_t highest_pdcp_sn) override
  // {
  //   mark_tx_handler->handle_transmit_notification(highest_pdcp_sn);
  // }

  // void on_delivery_notification(uint32_t highest_pdcp_sn) override
  // {
  //   mark_tx_handler->handle_delivery_notification(highest_pdcp_sn);
  // }

  // void on_retransmit_notification(uint32_t highest_pdcp_sn) override
  // {
  //   mark_tx_handler->handle_retransmit_notification(highest_pdcp_sn);
  // }

  // void on_delivery_retransmitted_notification(uint32_t highest_pdcp_sn) override
  // {
  //   mark_tx_handler->handle_delivery_retransmitted_notification(highest_pdcp_sn);
  // }

private:
  mark_rx_m1_interface* mark_rx_handler = nullptr;
  mark_tx_m1_interface* mark_tx_handler = nullptr;
};

/// Adapter between MR-U and M1 gateway adapter
class m1_gateway_rx_mru_adapter : public m1_cu_up_gateway_bearer_rx_notifier
{
public:
  m1_gateway_rx_mru_adapter()           = default;
  ~m1_gateway_rx_mru_adapter() override = default;

  void connect_mru_bearer(m1_rx_pdu_handler& m1_handler_) { m1_handler = &m1_handler_; }

  void on_new_pdu(mru_ul_buffer_state msg) override
  {
    srsran_assert(m1_handler != nullptr, "GTP-U handler must not be nullptr");
    m1_handler->handle_pdu(std::move(msg));
  }

private:
  m1_rx_pdu_handler* m1_handler = nullptr;
};

} // namespace srs_cu_up
} // namespace srsran
