#pragma once

#include "srsran/adt/byte_buffer_chain.h"

namespace srsran {
namespace srs_du {

/// \brief This interface represents the data entry point of the transmitting side of a F1-U bearer of the DU.
/// The lower layer will use this class to pass NR-U SDUs (e.g. PDCP PDUs/RLC SDUs) into the F1-U bearer towards CU-UP.
class m1_tx_sdu_handler
{
public:
  virtual ~m1_tx_sdu_handler() = default;

  virtual void handle_sdu(mru_ul_buffer_state ul_message) = 0;
};

} // namespace srs_du
} // namespace srsran
