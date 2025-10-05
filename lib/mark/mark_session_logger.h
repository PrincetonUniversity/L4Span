#pragma once 

#include "srsran/adt/optional.h"
#include "srsran/ran/cu_types.h"
#include "srsran/support/prefixed_logger.h"
#include "fmt/format.h"

namespace srsran{

class mark_session_trx_log_prefix
{
public:
  mark_session_trx_log_prefix(uint32_t                      ue_index,
                              pdu_session_id_t              psi,
                              const char*                   dir)
  {
    fmt::memory_buffer buffer;
    fmt::format_to(buffer, "ue={} {} {}: ", ue_index, psi, dir);
    prefix = srsran::to_c_str(buffer);
  }
  const char* to_c_str() const { return prefix.c_str(); }

private:
  std::string prefix;
};

using mark_session_trx_logger = prefixed_logger<mark_session_trx_log_prefix>;

class mark_session_log_prefix
{
public:
  mark_session_log_prefix(uint32_t ue_index, pdu_session_id_t psi)
  {
    fmt::memory_buffer buffer;
    fmt::format_to(buffer, "ue={} {}: ", ue_index, psi);
    prefix = srsran::to_c_str(buffer);
  }
  const char* to_c_str() const { return prefix.c_str(); }

private:
  std::string prefix;
};

using mark_session_logger = prefixed_logger<mark_session_log_prefix>;

} // namespace srsran

namespace fmt {

// associated formatters
template <>
struct formatter<srsran::mark_session_trx_log_prefix> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mark_session_trx_log_prefix o, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "{}", o.to_c_str());
  }
};

template <>
struct formatter<srsran::mark_session_log_prefix> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mark_session_log_prefix o, FormatContext& ctx) 
      -> decltype (std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "{}", o.to_c_str());
  }
};

} // namespace fmt