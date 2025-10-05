#pragma once

#include "fmt/format.h"
#include <cstdint>

namespace srsran::srs_du {

/// \brief Configurable parameters of the M1 bearer in the DU
struct m1_config {
  uint32_t t_notify;              ///< Timer used for periodic transmission of uplink notifications
  uint32_t rlc_queue_bytes_limit; ///< RLC queue limit in bytes. Used for initial report of buffer space to the CU-UP.
  bool     warn_on_drop = true;   ///< Log a warning instead of an info message whenever a PDU is dropped

  bool operator==(const m1_config& other) const
  {
    return t_notify == other.t_notify and rlc_queue_bytes_limit == other.rlc_queue_bytes_limit and
           warn_on_drop == other.warn_on_drop;
  }
};

} // namespace srsran::srs_du

namespace fmt {

// M1 config formatter
template <>
struct formatter<srsran::srs_du::m1_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::srs_du::m1_config cfg, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(),
                     "t_notify={} rlc_queue_bytes_limit={} warn_on_drop={}",
                     cfg.t_notify,
                     cfg.rlc_queue_bytes_limit,
                     cfg.warn_on_drop);
  }
};

} // namespace fmt
