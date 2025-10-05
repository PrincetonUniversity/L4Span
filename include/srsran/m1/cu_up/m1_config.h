#pragma once

#include "fmt/format.h"

namespace srsran {
namespace srs_cu_up {

/// \brief Configurable parameters of the M1 bearer in the CU-UP
struct m1_config {
  bool warn_on_drop = true; /// <Log a warning instead of an info message whenever a PDU is dropped
};

} // namespace srs_cu_up
} // namespace srsran

namespace fmt {

// M1 config formatter
template <>
struct formatter<srsran::srs_cu_up::m1_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::srs_cu_up::m1_config cfg, FormatContext& ctx)
    -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "warn_on_drop={}", cfg.warn_on_drop);
  }
};
} // namespace fmt