#pragma once

#include "srsran/adt/byte_buffer.h"

namespace srsran {
namespace srs_du {

/// \brief This interface represents the data exit point of the receiving side of a M1 bearer of the DU.
/// The M1 will push MR-U SDUs to the lower layer (e.g. RLC) using this interface.
class m1_rx_sdu_notifier
{
public:
  virtual ~m1_rx_sdu_notifier() = default;

  virtual void on_new_sdu(byte_buffer sdu) = 0;
};

} // namespace srs_du
} // namespace srsran
