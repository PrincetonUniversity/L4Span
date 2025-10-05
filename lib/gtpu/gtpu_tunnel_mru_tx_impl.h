#pragma once 

#include "gtpu_pdu.h"
#include "gtpu_tunnel_base_tx.h"
#include "srsran/adt/byte_buffer.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_tunnel_mru_tx.h"
#include "srsran/mru/mru_packing.h"
#include <arpa/inet.h>
#include <netinet/in.h>

namespace srsran {

/// Class used for transmitting GTP-U MR-U bearers, e.g. on M1 interface.
class gtpu_tunnel_mru_tx_impl : public gtpu_tunnel_base_tx, public gtpu_tunnel_mru_tx_lower_layer_interface
{
public:
  gtpu_tunnel_mru_tx_impl(srs_cu_up::ue_index_t                             ue_index,
                          gtpu_tunnel_mru_config::gtpu_tunnel_mru_tx_config cfg_,
                          dlt_pcap&                                         gtpu_pcap_,
                          gtpu_tunnel_common_tx_upper_layer_notifier&       upper_dn_) : 
    gtpu_tunnel_base_tx(gtpu_tunnel_log_prefix{ue_index, cfg_.peer_teid, "TX"}, gtpu_pcap_, upper_dn_),
    packer(logger.get_basic_logger()),
    cfg(cfg_)
  {
    to_sockaddr(peer_sockaddr, cfg.peer_addr.c_str(), cfg.peer_port);
    logger.log_info("GTPU MR-U Tx configured. {}", cfg);
  }

  /* 
  * SDU/PDU handler
  */
  void handle_sdu(mru_dl_message dl_message) final 
  {
    gtpu_header hdr         = {};
    hdr.flags.version       = GTPU_FLAGS_VERSION_V1;
    hdr.flags.protocol_type = GTPU_FLAGS_GTP_PROTOCOL;
    hdr.flags.ext_hdr       = true;
    hdr.message_type        = GTPU_MSG_DATA_PDU;
    hdr.length              = 0; // this will be computed automatically
    hdr.teid                = cfg.peer_teid;
    hdr.next_ext_hdr_type   = gtpu_extension_header_type::nr_ran_container;

    byte_buffer ext_buf;
    if (!packer.pack(ext_buf, dl_message)) {
      logger.log_error("Dropped SDU, error writing NR RAN MRU container to GTP-U extension header. teid={} ext_len={}",
                       hdr.teid,
                       ext_buf.length());
      return;
    }

    gtpu_extension_header ext;
    ext.extension_header_type = gtpu_extension_header_type::nr_ran_container;
    ext.container             = ext_buf;

    hdr.ext_list.push_back(ext);

    byte_buffer buf{std::move(dl_message.t_pdu)};
    bool write_ok = gtpu_write_header(buf, hdr, logger);

    if (!write_ok) {
      logger.log_error("Dropped SDU, error writing GTP-U header. teid={}", hdr.teid);
      return;
    }
    logger.log_info(buf.begin(), buf.end(), "TX PDU. pdu_len={} teid={}", buf.length(), hdr.teid);
    send_pdu(std::move(buf), peer_sockaddr);
  }

  void handle_sdu(mru_ul_buffer_state ul_message) final
  {
    gtpu_header hdr         = {};
    hdr.flags.version       = GTPU_FLAGS_VERSION_V1;
    hdr.flags.protocol_type = GTPU_FLAGS_GTP_PROTOCOL;
    hdr.flags.ext_hdr       = true;
    hdr.message_type        = GTPU_MSG_DATA_PDU;
    hdr.length              = 0; // this will be computed automatically
    hdr.teid                = cfg.peer_teid;
    hdr.next_ext_hdr_type   = gtpu_extension_header_type::no_more_extension_headers;

    logger.log_info("ul_message: drb_id {}, is_am {}, tx_sdu {}, tx_bytes {}, retx_sdu {}, retx_bytes {}",
      ul_message.drb_id,
      ul_message.is_am,
      ul_message.tx_sdu,
      ul_message.tx_bytes,
      ul_message.retx_sdu,
      ul_message.retx_bytes);

    byte_buffer ext_buf;
    if (!packer.pack(ext_buf, ul_message)) {
      logger.log_error("Dropped SDU, error writing NR RAN container to GTP-U extension header. teid={} ext_len={}",
                         hdr.teid,
                         ext_buf.length());
      return;
    }
    logger.log_debug("Adding data_delivery_status to PDU. teid={}", hdr.teid);
    hdr.next_ext_hdr_type = gtpu_extension_header_type::nr_ran_container;
    gtpu_extension_header ext;
    ext.extension_header_type = gtpu_extension_header_type::nr_ran_container;
    ext.container             = ext_buf;

    hdr.ext_list.push_back(ext);

    byte_buffer buf{std::move(ul_message.t_pdu)};
    bool write_ok = gtpu_write_header(buf, hdr, logger);

    if (!write_ok) {
      logger.log_error("Dropped SDU, error writing GTP-U header. teid={}", hdr.teid);
      return;
    }
    logger.log_info(buf.begin(), buf.end(), "TX PDU. pdu_len={} teid={}", buf.length(), hdr.teid);
    send_pdu(std::move(buf), peer_sockaddr);
  }

private:
  mru_packing packer;

  const gtpu_tunnel_mru_config::gtpu_tunnel_mru_tx_config cfg;
  sockaddr_storage                                        peer_sockaddr;
};

} // namespace srsran