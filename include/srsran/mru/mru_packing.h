#pragma once 

#include "srsran/adt/byte_buffer.h"
#include "srsran/mru/mru_message.h"
#include "srsran/srslog/logger.h"

namespace srsran {

enum class mru_pdu_type : uint8_t {
  mru_dl_message            = 0, ///< DL user data (PDU type 0)
  mru_ul_buffer_state       = 1, ///< UL buffer state (PDU type 1)
  reserved                  = 2  ///< Reserved for future use (PDU type 2)
};

/// Convert MR-U PDU type to unsigned integer.
constexpr inline uint8_t mru_pdu_type_to_uint(mru_pdu_type pdu_type)
{
  return static_cast<uint8_t>(pdu_type);
}

/// Convert unsigned integer to MR-U PDU type.
constexpr inline mru_pdu_type uint_to_mru_pdu_type(uint8_t num)
{
  return static_cast<mru_pdu_type>(num);
}

class mru_packing
{
public:
  mru_packing(srslog::basic_logger& logger_) : logger(logger_) {}

  static mru_pdu_type get_pdu_type(byte_buffer_view container);

  bool unpack(mru_dl_message& dl_user_data, byte_buffer_view container) const;
  bool pack(byte_buffer& out_buf, const mru_dl_message& dl_user_data) const;

  bool unpack(mru_ul_buffer_state& ul_message, byte_buffer_view container) const;
  bool pack(byte_buffer& out_buf, const mru_ul_buffer_state& ul_message) const;


private:
  srslog::basic_logger& logger;
};


} // namespace srsran

namespace fmt {

// MR-U PDU type field formatter
template <>
struct formatter<srsran::mru_pdu_type> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mru_pdu_type pdu_type, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    static constexpr const char* options[] = {"mru_dl_message", "mru_ul_buffer_state"};
    if (mru_pdu_type_to_uint(pdu_type) < mru_pdu_type_to_uint(srsran::mru_pdu_type::reserved)) {
      format_to(ctx.out(), "{}", options[mru_pdu_type_to_uint(pdu_type)]);
    }
    return format_to(ctx.out(), "reserved ({})", mru_pdu_type_to_uint(pdu_type));
  };
};
} // namespace fmt
