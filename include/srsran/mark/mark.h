#pragma once

#include "srsran/adt/byte_buffer.h"
#include "srsran/ran/cu_types.h"
#include "srsran/ran/lcid.h"
#include "srsran/mark/mark_m1_rx.h"
#include "srsran/mark/mark_m1_tx.h"
#include "srsran/mark/mark_config.h"
#include "srsran/mark/ip_utils.h"
#include "srsran/pdcp/pdcp_config.h"

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

namespace srsran {
namespace srs_cu_up {
// :TODO: These are the main input/output interfaces to pass data traffic 
// between layers. As the implementation progresses on we'll add additional 
// files that include new interfaces for configuration and other responsibilities.
// Please note these interfaces represent a single bearer or MARK entity.

/// This interface represents the entry point of the receiving side of a 
/// MARK entity.
class mark_rx_pdu_handler
{
public:
  virtual ~mark_rx_pdu_handler() = default;

  /// Handle the incoming PDU.
  virtual void handle_pdu(byte_buffer pdu, qos_flow_id_t qfi) = 0;

  /// @brief: TCP/IP flow to DRB mapping
  std::unordered_map<ip::five_tuple, ip::drb_tcp_state> five_tuple_to_drb;

  std::unordered_map<ip::five_tuple, ip::rtt_estimates> five_tuple_to_rtt;

  /// @brief: flow state inside each DRB, have L4S or classic (TCP/UDP) flows,
  /// as well as the marking decision.
  std::unordered_map<drb_id_t, mark_utils::drb_flow_state> drb_flow_state;
};

/// This interface notifies to upper layers the reception of new SDUs in 
/// the receiving side of a MARK entity.
class mark_rx_sdu_notifier
{
public:
  virtual ~mark_rx_sdu_notifier() = default;

  /// This callback is invoked on each generated SDU.
  virtual void on_new_sdu(byte_buffer sdu, qos_flow_id_t qfi) = 0;
};

/// This interface notifies to lower layers the generation of new PDUs 
/// in the transmitting side of a MARK entity.
class mark_tx_pdu_notifier
{
public:
  virtual ~mark_tx_pdu_notifier() = default;

  /// This callback is invoked on each generated PDU.
  virtual void on_new_pdu(byte_buffer pdu, qos_flow_id_t qfi) = 0;
};

/// This interface represents the entry point of the transmitting side of 
/// a MARK entity.
class mark_tx_sdu_handler
{
public:
  virtual ~mark_tx_sdu_handler() = default;

  /// Handle the incoming SDU.
  virtual void handle_sdu(byte_buffer sdu, qos_flow_id_t qos_flow_id) = 0;
};

/// This interface notifies to M1 interfaces something 
/// in the transmitting side of a MARK entity.
class mark_tx_m1_notifier
{
public:
  virtual ~mark_tx_m1_notifier() = default;

  /// This callback is invoked on each generated PDU.
  virtual void on_new_pdu(byte_buffer pdu) = 0;
};

/// Interface for the MARK entity.
/// Provides getters for the RX and TX parts of the MARK entity.
class mark_entity
{
public:
  mark_entity()          = default;
  virtual ~mark_entity() = default;

  virtual mark_rx_pdu_handler& get_mark_rx_pdu_handler()               = 0;
  virtual mark_tx_sdu_handler& get_mark_tx_sdu_handler()               = 0;
  // virtual mark_rx_m1_interface& get_mark_rx_m1_interface()              = 0;
  // virtual mark_tx_m1_interface& get_mark_tx_m1_interface()              = 0;
  virtual mark_rx_lower_interface& get_mark_rx_lower_interface()       = 0;
  virtual mark_tx_lower_interface& get_mark_tx_lower_interface()       = 0;

  virtual void create_tx (mark_tx_pdu_notifier& tx_pdu_notifier)       = 0;
  virtual void create_rx ()                                            = 0;
  virtual void add_drb(drb_id_t drb_id, pdcp_rlc_mode rlc_mod)         = 0;
  virtual void set_pdcp_sn_size(drb_id_t drb_id, uint8_t pdcp_sn_size) = 0;
  virtual void add_mapping(qos_flow_id_t qfi, drb_id_t drb_id)         = 0;

  size_t nof_ue = 0;

  // virtual bool is_mapped(qos_flow_id_t qfi) = 0;
  // virtual void
  // add_mapping(qos_flow_id_t qfi, drb_id_t drb_id, mark_config mark_cfg, 
  //   mark_tx_pdu_notifier& tx_pdu_notifier) = 0;
  // virtual void remove_mapping(drb_id_t drb_id) = 0;
};
} // namespace srs_cu_up
} // namespace srsran
