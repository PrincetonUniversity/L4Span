#pragma once

#include "srsran/adt/byte_buffer_chain.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This interface represents the data exit point of the receiving side of a M1 bearer of the CU-UP.
/// The M1 bearer will use this notifier to pass MR-U SDUs (e.g. RLC feedbacks) to upper layer (e.g. MARK).
class m1_rx_sdu_notifier
{
public:
  virtual ~m1_rx_sdu_notifier() = default;

  virtual void on_new_sdu(byte_buffer sdu, drb_id_t drb_id) = 0;
};

} // namespace srs_cu_up
} // namespace srsran