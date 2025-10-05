#include "srsran/mru/mru_packing.h"
#include "srsran/support/bit_encoding.h"

namespace srsran {

/// Macro used to check a read/unpack operation and log an error message if the validation fails.
#define VERIFY_READ(cond)                                                                                              \
  if (not(cond)) {                                                                                                     \
    logger.error("Read failed in {}", __FUNCTION__);                                                                   \
    return false;                                                                                                      \
  }

/// Macro used to check a write/append/pack operation and log an error message if the validation fails.
#define VERIFY_WRITE(cond)                                                                                             \
  if (not(cond)) {                                                                                                     \
    logger.error("Write failed in {}", __FUNCTION__);                                                                  \
    return false;                                                                                                      \
  }

mru_pdu_type mru_packing::get_pdu_type(byte_buffer_view container)
{
  srsran_assert(!container.empty(), "Cannot get PDU type of empty PDU");
  uint8_t field_value = container[0] >> 4;
  return uint_to_mru_pdu_type(field_value);
}

bool mru_packing::unpack(mru_dl_message& dl_message, byte_buffer_view container) const
{
  if (container.empty()) {
    logger.error("Failed to unpack DL user data: pdu_len=0");
    return false;
  }

  if ((container.length() + 2) % 4 != 0) {
    logger.error("Failed to unpack DL user data: pdu_len={} != n*4-2", container.length());
    return false;
  }

  bit_decoder decoder{container};

  // PDU Type
  uint8_t pdu_type = {};
  VERIFY_READ(decoder.unpack(pdu_type, 4));
  if (uint_to_mru_pdu_type(pdu_type) != mru_pdu_type::mru_dl_message) {
    logger.error("Failed to unpack DL user data: Invalid pdu_type={}", uint_to_mru_pdu_type(pdu_type));
    return false;
  }

  // DRB ID
  uint8_t drb_id = {};
  VERIFY_READ(decoder.unpack(drb_id, 8));

  dl_message.drb_id = static_cast<drb_id_t>(drb_id);

  return true;
};

bool mru_packing::pack(byte_buffer& out_buf, const mru_dl_message& dl_message) const
{
  size_t      start_len = out_buf.length();
  bit_encoder encoder{out_buf};

  // PDU Type
  VERIFY_WRITE(encoder.pack(mru_pdu_type_to_uint(mru_pdu_type::mru_dl_message), 4));

  // DRB ID
  VERIFY_WRITE(encoder.pack(static_cast<uint8_t>(dl_message.drb_id), 8));

  // Add padding such that length is (n*4-2) octets, where n is a positive integer.
  while (((out_buf.length() - start_len) + 2) % 4) {
    VERIFY_WRITE(out_buf.append(0x0));
  }

  return true;
};

bool mru_packing::unpack(mru_ul_buffer_state& ul_message, byte_buffer_view container) const
{
  if (container.empty()) {
    logger.error("Failed to unpack DL user data: pdu_len=0");
    return false;
  }

  bit_decoder decoder{container};
  logger.info("container size: {}", container.length());

  // PDU Type
  uint8_t pdu_type = {};
  VERIFY_READ(decoder.unpack(pdu_type, 4));
  // pdu_type = container[0] >> 4;
  if (uint_to_mru_pdu_type(pdu_type) != mru_pdu_type::mru_ul_buffer_state) {
    logger.error("Failed to unpack UL buffer state: Invalid pdu_type={}, {}", uint_to_mru_pdu_type(pdu_type), pdu_type);
    return false;
  }

  // DRB_ID
  uint8_t drb_id = {};
  VERIFY_READ(decoder.unpack(drb_id, 8));

  // AM or UM
  bool is_am = {};
  VERIFY_READ(decoder.unpack(is_am, 1));

  // TX SDU 
  uint32_t tx_sdu = {};
  VERIFY_READ(decoder.unpack(tx_sdu, 32)); 

  // TX Bytes 
  uint32_t tx_bytes = {};
  VERIFY_READ(decoder.unpack(tx_bytes, 32)); 

  // ReTX SDU 
  uint32_t retx_sdu = {};
  VERIFY_READ(decoder.unpack(retx_sdu, 32)); 

  // ReTX Bytes 
  uint32_t retx_bytes = {};
  VERIFY_READ(decoder.unpack(retx_bytes, 32)); 

  ul_message.drb_id = static_cast<drb_id_t>(drb_id);
  ul_message.is_am = is_am;
  ul_message.tx_sdu = tx_sdu;
  ul_message.tx_bytes = tx_bytes;
  ul_message.retx_sdu = retx_sdu;
  ul_message.retx_bytes = retx_bytes;

  return true;
};

bool mru_packing::pack(byte_buffer& out_buf, const mru_ul_buffer_state& ul_message) const
{
  size_t      start_len = out_buf.length();
  bit_encoder encoder{out_buf};

  // PDU Type
  VERIFY_WRITE(encoder.pack(mru_pdu_type_to_uint(mru_pdu_type::mru_ul_buffer_state), 4));

  // DRB_ID
  VERIFY_WRITE(encoder.pack(static_cast<uint8_t>(ul_message.drb_id), 8));

  // AM or UM
  VERIFY_WRITE(encoder.pack(ul_message.is_am, 1));

  // TX SDU
  VERIFY_WRITE(encoder.pack(ul_message.tx_sdu, 32));

  // TX Bytes
  VERIFY_WRITE(encoder.pack(ul_message.tx_bytes, 32));

  // ReTX SDU
  VERIFY_WRITE(encoder.pack(ul_message.retx_sdu, 32));

  // ReTX Bytes
  VERIFY_WRITE(encoder.pack(ul_message.retx_bytes, 32));

  // Add padding such that length is (n*4-2) octets, where n is a positive integer.
  while (((out_buf.length() - start_len) + 2) % 4) {
    VERIFY_WRITE(out_buf.append(0x0));
  }

  return true;
};

} // namespace srsran