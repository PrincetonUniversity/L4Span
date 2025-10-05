#pragma once 

#include "srsran/nru/nru_message.h"

namespace srsran {
namespace srs_cu_up{

/// \brief This interface represents the MR-U exit point of the M1 bearer of the CU-UP
/// through which it passes MR-U PDUs to lower layer (e.g. RLC) towards the DU.
class m1_tx_pdu_notifier
{
public:
  virtual ~m1_tx_pdu_notifier() = default;

  virtual void on_new_pdu(mru_dl_message msg) = 0;
};

} // namespace srs_cu_up
} // namespace srsran