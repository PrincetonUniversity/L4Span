#pragma once

#include "gtpu_tunnel_base_rx.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_tunnel_mru_rx.h"
#include "srsran/mru/mru_packing.h"

namespace srsran {

/// Class used for receiving GTP-U MR-U bearers, e.g. on M1 interface.
class gtpu_tunnel_mru_rx_impl : public gtpu_tunnel_base_rx
{
public:
  gtpu_tunnel_mru_rx_impl(srs_cu_up::ue_index_t                             ue_index,
                          gtpu_tunnel_mru_config::gtpu_tunnel_mru_rx_config cfg,
                          gtpu_tunnel_mru_rx_lower_layer_notifier&          rx_lower_) :
    gtpu_tunnel_base_rx(gtpu_tunnel_log_prefix{ue_index, cfg.local_teid, "RX"}),
    packer(logger.get_basic_logger()),
    lower_dn(rx_lower_),
    config(cfg)
  {
    logger.log_info("GTPU MR-U Rx configured. local_teid={}", config.local_teid);
  }
  ~gtpu_tunnel_mru_rx_impl() override = default;

protected:
  // domain-specific PDU handler
  void handle_pdu(gtpu_dissected_pdu&& pdu, const sockaddr_storage& src_addr) final
  {
    size_t                 pdu_len = pdu.buf.length();
    gtpu_teid_t            teid    = pdu.hdr.teid;
    std::variant<mru_dl_message, mru_ul_buffer_state> mru_msg;
    bool                   have_nr_ran_container = false;
    // std::variant<mru_dl_message, mru_ul_buffer_state> mru_msg;
    for (auto ext_hdr : pdu.hdr.ext_list) {
      switch (ext_hdr.extension_header_type) {
        case gtpu_extension_header_type::nr_ran_container:
          if (!have_nr_ran_container) {
            mru_pdu_type pdu_type = packer.get_pdu_type(ext_hdr.container);
            switch (pdu_type) {
              case mru_pdu_type::mru_dl_message:
                mru_msg               = {mru_dl_message{}};
                have_nr_ran_container = packer.unpack(std::get<mru_dl_message>(mru_msg), ext_hdr.container);
                break;
              case mru_pdu_type::mru_ul_buffer_state:
                mru_msg               = {mru_ul_buffer_state{}};
                have_nr_ran_container = packer.unpack(std::get<mru_ul_buffer_state>(mru_msg), ext_hdr.container);
                break;
              default:
                logger.log_warning(
                    "Unsupported PDU type in NR RAN container. pdu_type={} pdu_len={}", pdu_type, pdu_len);
                return;
            }
            if (!have_nr_ran_container) {
              logger.log_error("Failed to unpack NR RAN container. pdu_len={}", pdu_len);
            }
          } else {
            logger.log_warning("Ignoring multiple NR RAN container. pdu_len={}", pdu_len);
          }
          break;
        default:
          logger.log_warning("Ignoring unexpected extension header at F1-U interface. type={} pdu_len={}",
                             ext_hdr.extension_header_type,
                             pdu_len);
      }
    }
    if (!have_nr_ran_container) {
      logger.log_info("T-PDU without NR RAN container. Assuming DL. pdu_len={} teid={}", pdu_len, teid);
      mru_msg = {mru_dl_message{}}; // set to DL
    }

    logger.log_debug(pdu.buf.begin(), pdu.buf.end(), "RX PDU. pdu_len={}", pdu_len);

    if (std::holds_alternative<mru_ul_buffer_state>(mru_msg)) {
      mru_ul_buffer_state ul_message = {};
      ul_message = std::move(std::get<mru_ul_buffer_state>(mru_msg));
      ul_message.t_pdu = gtpu_extract_msg(std::move(pdu)); // header is invalidated after extraction;
      logger.log_info(
        ul_message.t_pdu.begin(), ul_message.t_pdu.end(), 
        "RX UL buffer state feedback, teid {}. is_am {}, tx_sdu {}, tx_bytes {}, t_pdu_len={}", 
        teid,
        ul_message.is_am,
        ul_message.tx_sdu,
        ul_message.tx_bytes,
        ul_message.t_pdu.length());
      lower_dn.on_new_sdu(std::move(ul_message));
      return;
    }

    // We should never come here
    logger.log_error("Unhandled MR-U PDU. pdu_len={}", pdu_len);
  }

private:
  mru_packing                              packer;
  gtpu_tunnel_mru_rx_lower_layer_notifier& lower_dn;

  /// Rx config
  gtpu_tunnel_mru_config::gtpu_tunnel_mru_rx_config config;
};
} // namespace srsran
