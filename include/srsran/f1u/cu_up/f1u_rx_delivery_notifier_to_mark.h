#pragma once

#include <cstdint>
#include "srsran/mark/mark.h"
#include "srsran/ran/lcid.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This interface represents the notification exit point of the receiving side of a F1-U bearer of the CU-UP
/// through which the lower layer (e.g. RLC in the DU) notifies the MARK Tx entity of transmit/delivery of PDCP PDUs.
class f1u_rx_delivery_notifier_to_mark
{
public:
  virtual ~f1u_rx_delivery_notifier_to_mark() = default;

  virtual void on_delivery_status(mark_utils::delivery_status_feedback feedback, drb_id_t drb_id_) = 0;
  // virtual void on_transmit_notification(uint32_t highest_pdcp_sn) = 0;
  // virtual void on_delivery_notification(uint32_t highest_pdcp_sn, drb_id_t drb_id_) = 0;
  // virtual void on_retransmit_notification(uint32_t highest_pdcp_sn) = 0;
  // virtual void on_delivery_retransmitted_notification(uint32_t highest_pdcp_sn) = 0;
};

} // namespace srs_cu_up
} // namespace srsran
