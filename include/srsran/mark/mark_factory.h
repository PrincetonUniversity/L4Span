#pragma once 

#include "srsran/ran/cu_types.h"
#include "srsran/mark/mark.h"
#include "srsran/support/timers.h"

/// This factory header file depends on the MARK interfaces. 
/// It is kept separate as clients of the MARK interfaces don't need to call 
/// factory methods.

namespace srsran {

namespace srs_cu_up {

struct mark_entity_creation_message {
  uint32_t              ue_index;
  pdu_session_id_t      pdu_session_id;
  mark_rx_sdu_notifier* rx_sdu_notifier;
  size_t                nof_drbs;
};

/// Creates an interface of a MARK interface.
std::unique_ptr<mark_entity> create_mark(mark_entity_creation_message& msg);

} // namespace srs_cu_up

} // namespace srsran
