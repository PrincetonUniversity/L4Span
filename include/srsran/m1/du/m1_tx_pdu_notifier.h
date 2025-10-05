#pragma once

#include "srsran/mru/mru_message.h"

namespace srsran {
namespace srs_du {

/// \brief This interface represents the MR-U exit point of a F1-U bearer of the DU.
/// The F1-U bearer will use this notifier to pass NR-U PDUs to upper layer (e.g. GTP-U) towards CU-UP.
class m1_tx_pdu_notifier
{
public:
  virtual ~m1_tx_pdu_notifier() = default;

  virtual void on_new_pdu(mru_ul_buffer_state msg) = 0;
};

} // namespace srs_du
} // namespace srsran
