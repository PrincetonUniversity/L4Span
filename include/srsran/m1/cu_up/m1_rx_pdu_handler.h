#pragma once

#include "srsran/mru/mru_message.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This interface represents the MR-U entry point of a M1 bearer of the CU-UP.
/// The lower layer (e.g. GTP-U) will use this class to pass MR-U PDUs (from the DU) into the M1.
class m1_rx_pdu_handler
{
public:
  virtual ~m1_rx_pdu_handler() = default;

  // Use the nru_ul_message temporarily
  virtual void handle_pdu(mru_ul_buffer_state msg) = 0;
};

} // namespace srs_cu_up
} // namespace srsran