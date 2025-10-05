#pragma once

#include "srsran/gtpu/gtpu_tunnel_ngu_tx.h"
#include "srsran/pdcp/pdcp_tx.h"
#include "srsran/sdap/sdap.h"
#include "srsran/mark/mark.h"
#include "srsran/m1/cu_up/m1_tx_sdu_handler.h"

namespace srsran {

namespace srs_cu_up {

/// Adapter between MARK and GTP-U
class mark_gtpu_adapter : public mark_rx_sdu_notifier
{
public:
  mark_gtpu_adapter()  = default;
  ~mark_gtpu_adapter() = default;

  void connect_gtpu(gtpu_tunnel_ngu_tx_lower_layer_interface& gtpu_handler_)
  {
    gtpu_handler = &gtpu_handler_;
  }

  void on_new_sdu(byte_buffer sdu, qos_flow_id_t qfi) override
  {
    srsran_assert(gtpu_handler != nullptr, "GTPU handler must not be nullptr");
    gtpu_handler->handle_sdu(std::move(sdu), qfi);
  }

private:
  gtpu_tunnel_ngu_tx_lower_layer_interface* gtpu_handler = nullptr;
};

class mark_sdap_adapter : public mark_tx_pdu_notifier
{
public:
  mark_sdap_adapter()  = default;
  ~mark_sdap_adapter() = default;

  void connect_sdap(sdap_tx_sdu_handler& sdap_handler_)
  {
    sdap_handler = &sdap_handler_;
  }
  void disconnect_sdap() { sdap_handler = nullptr; }

  void on_new_pdu(byte_buffer pdu, qos_flow_id_t qfi) override
  {
    if (sdap_handler == nullptr) {
      srslog::fetch_basic_logger("MARK").warning(
        "Unconnected SDAP handler. Dropping MARK PDU");
    } else {
      sdap_handler->handle_sdu(std::move(pdu), qfi);
    }
  }

private:
  sdap_tx_sdu_handler* sdap_handler = nullptr;
};

class mark_m1_adapter : public mark_tx_m1_notifier
{
public:
  mark_m1_adapter()  = default;
  ~mark_m1_adapter() = default;

  void connect_m1(m1_tx_sdu_handler& m1_handler_) { m1_handler = &m1_handler_;}
  void disconnect_m1() { m1_handler = nullptr; }

  void on_new_pdu(byte_buffer pdu) override
  {
    if (m1_handler == nullptr) {
      srslog::fetch_basic_logger("MARK").info("Dropped DL PDU. M1 handler is not connected.");
    } else {
      m1_handler->handle_sdu(std::move(pdu));
    }
  }

private:
  m1_tx_sdu_handler* m1_handler = nullptr;
};

} // namespace srs_cu_up

} // namespace srsran