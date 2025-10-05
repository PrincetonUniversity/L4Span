#pragma once 

#include "mark_session_logger.h"
#include "srsran/mark/mark.h"

#include <sstream>
#include <iomanip>
#include <map>

namespace srsran {

namespace srs_cu_up {
  
class mark_entity_tx_impl
{
public:
  mark_entity_tx_impl(uint32_t              ue_index,
                      pdu_session_id_t      psi,
                      mark_tx_pdu_notifier& pdu_notifier_) :
    logger("MARK", {ue_index, psi, "DL"}), 
    pdu_notifier(pdu_notifier_)
  {
  }

  void handle_sdu(byte_buffer sdu, qos_flow_id_t qfi_)
  { 
    pdu_notifier.on_new_pdu(std::move(sdu), qfi_);
  }

private:
  mark_session_trx_logger logger;
  mark_tx_pdu_notifier&   pdu_notifier;
};

} // namespace srs_cu_up
} // namespace srsran
