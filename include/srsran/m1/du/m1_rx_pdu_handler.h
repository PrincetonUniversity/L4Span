#pragma once

#include "srsran/mru/mru_message.h"

namespace srsran {
namespace srs_du {

/// \brief This interface represents the NR-U entry point of the receiving side of a F1-U bearer of the DU.
/// The upper layer (e.g. GTP-U) will use it to pass NR-U PDUs (from the CU-UP) into the F1-U bearer of the DU.
class m1_rx_pdu_handler
{
public:
  virtual ~m1_rx_pdu_handler() = default;

  virtual void handle_pdu(mru_dl_message msg) = 0;
};

} // namespace srs_du
} // namespace srsran
