#pragma once 

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/byte_buffer_chain.h"
#include "srsran/adt/static_vector.h"
#include "srsran/ran/lcid.h"

#include <cstdint>
#include <optional>

namespace srsran {

/// MR-U DL message from the MARK entity to lower layer (RLC), 
/// not sure what to include right now. Just a place holder.
struct mru_dl_message {
  // To be defined, 
  drb_id_t drb_id;

  byte_buffer t_pdu;

  expected<mru_dl_message> deep_copy()
  {
    mru_dl_message       copy = {};
    expected<byte_buffer> buf = t_pdu.deep_copy();
    if (not buf.has_value()) {
      return make_unexpected(default_error_t{});
    }
    copy.t_pdu = std::move(buf.value());
    return copy;
  }

  bool operator==(const mru_dl_message& other) const
  {
    return drb_id == other.drb_id && t_pdu == other.t_pdu;
  }
  bool operator!=(const mru_dl_message& other) const { return not(*this == other); }
};

/// MR-U UL message from the RLC to the MARK layer or SDAP layer
struct mru_ul_buffer_state {
  drb_id_t drb_id;
  bool is_am;
  // Number of SDUs (packets) in the RLC DRB tx queue
  uint32_t tx_sdu;
  // Number of bytes in the RLC DRB tx queue
  uint32_t tx_bytes;
  // Number of SDUs (packets) in the RLC AM DRB retx queue
  uint32_t retx_sdu;
  // Number of bytes in the RCL AM DRB retx queue
  uint32_t retx_bytes;
  // Some other message to be defined
  byte_buffer t_pdu;

  expected<mru_ul_buffer_state> deep_copy()
  {
    mru_ul_buffer_state copy = {};
    expected<byte_buffer> buf = t_pdu.deep_copy();
    if (not buf.has_value()) {
      return make_unexpected(default_error_t{});
    }
    copy.t_pdu = std::move(buf.value());
    copy.drb_id = drb_id;
    copy.is_am = is_am;
    copy.tx_sdu = tx_sdu;
    copy.tx_bytes = tx_bytes;
    copy.retx_sdu = retx_sdu;
    copy.retx_bytes = retx_bytes;
    return copy;
  }

  bool operator==(const mru_ul_buffer_state& other) const
  {
    return drb_id == other.drb_id && is_am == other.is_am && tx_sdu == other.tx_sdu &&
           tx_bytes == other.tx_bytes && retx_sdu == other.retx_sdu && 
           retx_bytes == other.retx_bytes && t_pdu == other.t_pdu;
  }

  bool operator!=(const mru_ul_buffer_state& other) const { return not(*this == other); }
};

}