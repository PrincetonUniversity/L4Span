#pragma once 

#include "srsran/ran/lcid.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/prefixed_logger.h"
#include "fmt/format.h"
#include <string.h>

namespace srsran {
namespace srs_cu_up {

class m1_bearer_log_prefix
{
public:
  m1_bearer_log_prefix(uint32_t ue_index, drb_id_t drb_id, const up_transport_layer_info& ul_tnl_info)
  {
    fmt::memory_buffer buffer;
    fmt::format_to(buffer, "ue={} {} ul-teid={}: ", ue_index, drb_id, ul_tnl_info.gtp_teid);
    prefix = srsran::to_c_str(buffer);
  }
  const char* to_c_str() const { return prefix.c_str(); }

private:
  std::string prefix;
};

using m1_bearer_logger = prefixed_logger<m1_bearer_log_prefix>;

} // namespace srs_cu_up
} // namespace srsran

namespace fmt {

// associated formatter
template <>
struct formatter<srsran::srs_cu_up::m1_bearer_log_prefix> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::srs_cu_up::m1_bearer_log_prefix o, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "{}", o.to_c_str());
  }
};

}