#pragma once

#include "srsran/nru/nru_message.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This interface represents the NR-U entry point of a F1-U bearer of the CU-UP.
/// The lower layer (e.g. GTP-U) will use this class to pass NR-U PDUs (from the DU) into the F1-U.
class f1u_rx_pdu_mark_handler
{
public:
  virtual ~f1u_rx_pdu_mark_handler() = default;

  virtual void handle_pdu_to_mark(nru_ul_message msg) = 0;
};

} // namespace srs_cu_up
} // namespace srsran