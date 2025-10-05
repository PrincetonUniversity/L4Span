#pragma once

#include "adapters/m1_adapters.h"
#include "adapters/gw_adapters.h"
#include "srsran/m1/cu_up/m1_config.h"
#include "adapters/mark_adapters.h"
#include "srsran/ran/lcid.h"
#include <map>

namespace srsran {
namespace srs_cu_up {

/// \brief DRB context with map to all QoS flows.
struct mark_context {
  mark_context(const drb_id_t& drb_id_) : drb_id(drb_id_){};

  void stop()
  {
    m1->stop();
    m1_gw_bearer->stop();
  }

  drb_id_t    drb_id;
  gtpu_teid_t m1_ul_teid;
  m1_config   m1_cfg;

  std::unique_ptr<m1_cu_up_gateway_bearer> m1_gw_bearer;
  std::unique_ptr<m1_bearer>               m1;

  // Adapter M1->MARK
  mark_m1_adapter  mark_to_m1_adapter;
  m1_mark_adapter  m1_to_mark_adapter;

  // Adapter M1 gateway -> MR-U
  m1_gateway_rx_mru_adapter m1_gateway_rx_to_mru_adapter;

  uint8_t cell_group_id; /// This can/should be a list of cell groups.
};

} // namespace srs_cu_up
} // namespace srsran
