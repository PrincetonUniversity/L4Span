#pragma once 

#include "srsran/adt/byte_buffer_chain.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This interface represents the data entry point of the transmitting side of a M1 bearer of the CU-UP.
/// The upper layer (e.g. MARK) may push MR-U SDUs (e.g. MARK request?) into the M1 bearer towards the DU.
/// The upper layer (e.g. MARK) will also inform the lower layer of SDUs (e.g. RLC request) to be discarded?
/// TODO: Evaluate whether this is needed or not
class m1_tx_sdu_handler
{
public:
  virtual ~m1_tx_sdu_handler() = default;
  
  /// \brief Immediately transmits a MARK TX request to lower layers towards the DU.
  /// \param sdu The MARK TX request to be transmitted to lower layers, maybe not necessary
  virtual void handle_sdu(byte_buffer sdu) = 0;

  /// \brief Enqueues a notification to discard the given PDCP TX PDU at the DU.
  ///
  /// The queued notifications are either piggy-backed with the next SDU or sent periodically (whatever comes first).
  /// \param pdcp_sn The sequence number of the PDCP PDU that shall be discarded by lower layers.
  virtual void discard_sdu(uint32_t pdcp_sn) = 0;
};

} // namespace srs_cu_up
} // namespace srsran