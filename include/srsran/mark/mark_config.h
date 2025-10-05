#pragma once 

#include "fmt/format.h"
#include <cstdint>
#include <memory>
#include <string>

namespace srsran {

/// MARK header options for UL
enum class mark_hdr_ul_cfg { present = 0, absent };

/// MARK header options for DL
enum class mark_hdr_dl_cfg { present = 0, absent };

/// Configurable parameters for MARK mapping
struct mark_config {
  bool        default_drb   = false;
  mark_hdr_ul_cfg header_ul = mark_hdr_ul_cfg::absent;
  mark_hdr_dl_cfg header_dl = mark_hdr_dl_cfg::absent;
};

} // namespace srsran

// 
// Formatters
// 
namespace fmt {

// Header config
template <>
struct formatter<srsran::mark_hdr_ul_cfg> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mark_hdr_ul_cfg hdr_cfg, FormatContext& ctx)
    -> decltype(std::declval<FormatContext>().out())
  {
    static constexpr const char* options[] = {"present", "absent"};
    return format_to(ctx.out(), "{}", options[static_cast<unsigned>(hdr_cfg)]);
  }
};

// Header config
template <>
struct formatter<srsran::mark_hdr_dl_cfg> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mark_hdr_dl_cfg hdr_cfg, FormatContext& ctx)
    ->decltype(std::declval<FormatContext>().out())
  {
    static constexpr const char* options[] = {"present", "absent"};
    return format_to(ctx.out(), "{}", options[static_cast<unsigned>(hdr_cfg)]);
  }
};

// MARK config
template <>
struct formatter<srsran::mark_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mark_config cfg, FormatContext& ctx)
    -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "default_drb={} hdr_ul={} hdr_dl={}", 
      cfg.default_drb, cfg.header_ul, cfg.header_dl);
  }
};

} // namespace fmt