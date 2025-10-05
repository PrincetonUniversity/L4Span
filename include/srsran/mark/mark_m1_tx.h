#pragma once

#include "srsran/adt/byte_buffer.h"
#include "srsran/security/security.h"
#include "srsran/mark/mark_utils.h"

/*
 * This file will hold the interfaces and notifiers for the PDCP entity.
 * They follow the following nomenclature:
 *
 *   pdcp_{tx/rx}_{lower/upper}_{[control/data]}_{interface/notifier}
 *
 * 1. TX/RX indicates whether the interface is intended for the
 *    TX or RX side of the entity
 * 2. Lower/Upper indicates whether the interface/notifier interacts
 *    with the upper or lower layers.
 * 3. Control/Data: indicates whether this interface is necessary for "control"
 *    purposes (e.g., notifying the RRC of an integrity failure, or that we are
 *    near max HFN) or "data" purposes (e.g. handling SDUs).
 *    This distinction is only necessary when interfacing with the upper layers,
 *    and as such, we omit it in the interfaces with the lower layers.
 * 4. Interface/Notifier: whether this is an interface the PDCP entity will
 *    inherit or if a notifier that the PDCP will keep as a member.
 *
 */
namespace srsran {

// /// This interface represents the data exit point of the transmitting side of a PDCP entity.
// /// The PDCP will push PDUs to the lower layers using this interface.
// /// The PDCP will also use this interface to order the lower layer to discard PDUs if necessary.
// class pdcp_tx_lower_notifier
// {
// public:
//   pdcp_tx_lower_notifier()                                          = default;
//   virtual ~pdcp_tx_lower_notifier()                                 = default;
//   pdcp_tx_lower_notifier(const pdcp_tx_lower_notifier&)             = delete;
//   pdcp_tx_lower_notifier& operator=(const pdcp_tx_lower_notifier&)  = delete;
//   pdcp_tx_lower_notifier(const pdcp_tx_lower_notifier&&)            = delete;
//   pdcp_tx_lower_notifier& operator=(const pdcp_tx_lower_notifier&&) = delete;

//   virtual void on_new_pdu(byte_buffer pdu, bool is_retx) = 0; ///< Pass PDCP PDU to the lower layers.
//   virtual void on_discard_pdu(uint32_t pdcp_sn)          = 0; ///< Order lower layers to discard PDU
// };

/// This interface represents the notification entry point of the transmitting side of a MARK entity.
/// The lower layers will use this interface to inform the MARK Tx about RLC tx and retx buffer occupancy.
/// The MARK uses this information to mark the packet's IP headers to notify the TCP endpoint for congestion feecback:
class mark_tx_m1_interface
{
public:
  mark_tx_m1_interface()                                           = default;
  virtual ~mark_tx_m1_interface()                                  = default;
  mark_tx_m1_interface(const mark_tx_m1_interface&)             = delete;
  mark_tx_m1_interface& operator=(const mark_tx_m1_interface&)  = delete;
  mark_tx_m1_interface(const mark_tx_m1_interface&&)            = delete;
  mark_tx_m1_interface& operator=(const mark_tx_m1_interface&&) = delete;

  // /// \brief Informs the PDCP entity about the highest PDCP PDU sequence number of the PDCP PDU that was transmitted by
  // /// the lower layers (i.e. by the RLC).
  // ///
  // /// In case of RLC AM, the PDCP will ignore this notification.
  // /// In case of RLC UM, the PDCP will stop the discard timers for all PDUs up to highest_sn.
  // ///
  // /// \param highest_sn Highest transmitted PDCP PDU sequence number.
  // virtual void handle_transmit_notification(uint32_t highest_sn) = 0;

  // /// \brief Informs the PDCP about the highest PDCP PDU sequence number of the PDCP PDU that was successfully
  // /// delivered in sequence towards the UE.
  // ///
  // /// In case of RLC AM, the PDCP will stop the discard timers for all PDUs up to highest_sn.
  // /// In case of RLC UM, the PDCP will ignore this notification.
  // ///
  // /// \param highest_sn Highest in a sequence delivered PDCP PDU sequence number.
  // virtual void handle_delivery_notification(uint32_t highest_sn) = 0;

  // /// \brief Informs the PDCP entity about the highest PDCP PDU sequence number of the PDCP PDU that was retransmitted
  // /// by the lower layers (i.e. by the RLC AM).
  // ///
  // /// This notification is only applicable for RLC AM.
  // ///
  // /// \param highest_sn Highest retransmitted PDCP PDU sequence number.
  // virtual void handle_retransmit_notification(uint32_t highest_sn) = 0;

  // /// \brief Informs the PDCP about the highest PDCP PDU sequence number of the retransmitted PDCP PDU that was
  // /// successfully delivered in sequence towards the UE.
  // ///
  // /// This notification is only applicable for RLC AM.
  // ///
  // /// \param highest_sn Highest in a sequence delivered retransmitted PDCP PDU sequence number.
  // virtual void handle_delivery_retransmitted_notification(uint32_t highest_sn) = 0;
};

// /// This interface represents the data entry point of the transmitting side of a PDCP entity.
// /// The upper-layers will use this call to pass PDUs into the TX entity.
// class pdcp_tx_upper_data_interface
// {
// public:
//   pdcp_tx_upper_data_interface()                                                = default;
//   virtual ~pdcp_tx_upper_data_interface()                                       = default;
//   pdcp_tx_upper_data_interface(const pdcp_tx_upper_data_interface&)             = delete;
//   pdcp_tx_upper_data_interface& operator=(const pdcp_tx_upper_data_interface&)  = delete;
//   pdcp_tx_upper_data_interface(const pdcp_tx_upper_data_interface&&)            = delete;
//   pdcp_tx_upper_data_interface& operator=(const pdcp_tx_upper_data_interface&&) = delete;

//   /// Handle the incoming SDU.
//   virtual void handle_sdu(byte_buffer sdu) = 0;
// };

// /// This interface represents the control SAP of the transmitting side of a PDCP entity.
// /// The RRC will use this interface to configure security keys and enable/disable
// /// integrity and ciphering.
// class pdcp_tx_upper_control_interface
// {
// public:
//   pdcp_tx_upper_control_interface()                                                   = default;
//   virtual ~pdcp_tx_upper_control_interface()                                          = default;
//   pdcp_tx_upper_control_interface(const pdcp_tx_upper_control_interface&)             = delete;
//   pdcp_tx_upper_control_interface& operator=(const pdcp_tx_upper_control_interface&)  = delete;
//   pdcp_tx_upper_control_interface(const pdcp_tx_upper_control_interface&&)            = delete;
//   pdcp_tx_upper_control_interface& operator=(const pdcp_tx_upper_control_interface&&) = delete;

//   /// Setup security
//   virtual void configure_security(security::sec_128_as_config sec_cfg,
//                                   security::integrity_enabled integrity_enabled_,
//                                   security::ciphering_enabled ciphering_enabled_) = 0;

//   /// Trigger data recovery
//   virtual void data_recovery() = 0;

//   /// Trigger re-establishment
//   virtual void reestablish(security::sec_128_as_config sec_cfg) = 0;
// };

// /// This interface represents the control upper layer that the
// /// TX PDCP bearer must notify in case of reaching max HFN,
// /// so that keys can be re-negotiated. Other protocol failures
// /// will also be notified through this interface.
// class pdcp_tx_upper_control_notifier
// {
// public:
//   pdcp_tx_upper_control_notifier()                                                  = default;
//   virtual ~pdcp_tx_upper_control_notifier()                                         = default;
//   pdcp_tx_upper_control_notifier(const pdcp_tx_upper_control_notifier&)             = delete;
//   pdcp_tx_upper_control_notifier& operator=(const pdcp_tx_upper_control_notifier&)  = delete;
//   pdcp_tx_upper_control_notifier(const pdcp_tx_upper_control_notifier&&)            = delete;
//   pdcp_tx_upper_control_notifier& operator=(const pdcp_tx_upper_control_notifier&&) = delete;

//   virtual void on_protocol_failure()  = 0;
//   virtual void on_max_count_reached() = 0;
// };

class mark_tx_lower_interface
{
public:
  mark_tx_lower_interface()                                           = default;
  virtual ~mark_tx_lower_interface()                                  = default;
  mark_tx_lower_interface(const mark_tx_lower_interface&)             = delete;
  mark_tx_lower_interface& operator=(const mark_tx_lower_interface&)  = delete;
  mark_tx_lower_interface(const mark_tx_m1_interface&&)               = delete;
  mark_tx_lower_interface& operator=(const mark_tx_lower_interface&&) = delete;

  ///< Handle the incoming PDU.
  virtual void handle_feedback(mark_utils::delivery_status_feedback feedback, drb_id_t drb_id_) = 0; 
};
} // namespace srsran
