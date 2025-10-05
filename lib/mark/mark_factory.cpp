#include "srsran/mark/mark_factory.h"
#include "mark_entity_impl.h"

/// Notice this would be the only place were we include concrete class implementation files.

using namespace srsran;
using namespace srs_cu_up;

/// Factories are at a low level point of abstraction, as such, they are the "only" (best effort) objects that depend on
/// concrete class implementations instead of interfaces, intrinsically giving them tight coupling to the objects being
/// created. Keeping this coupling in a single file, is the best, as the rest of the code can be kept decoupled.

std::unique_ptr<mark_entity> srsran::srs_cu_up::create_mark(
  mark_entity_creation_message& msg)
{
  return std::make_unique<mark_entity_impl>(msg.ue_index, 
    msg.pdu_session_id, *msg.rx_sdu_notifier, msg.nof_drbs);
}