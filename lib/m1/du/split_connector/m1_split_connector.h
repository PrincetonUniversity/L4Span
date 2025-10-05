#pragma once 

#include "srsran/m1/du/m1_bearer_logger.h"
#include "srsran/m1/du/m1_gateway.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/gtpu_tunnel_common_tx.h"
#include "srsran/gtpu/gtpu_tunnel_mru.h"
#include "srsran/gtpu/gtpu_tunnel_mru_factory.h"
#include "srsran/gtpu/gtpu_tunnel_mru_rx.h"
#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/srslog/srslog.h"
#include <cstdint>
#include <unordered_map>

namespace srsran::srs_du {

class gtpu_tx_udp_gw_adapter : public gtpu_tunnel_common_tx_upper_layer_notifier
{
public:
  /// \brief Interface for the GTP-U to pass PDUs to the IO gateway
  /// \param sdu PDU to be transmitted
  void on_new_pdu(byte_buffer buf, const sockaddr_storage& addr) override
  {
    if (handler != nullptr) {
      handler->handle_pdu(std::move(buf), addr);
    }
  }

  void connect(srs_cu_up::ngu_tnl_pdu_session& handler_) { handler = &handler_; }

  void disconnect() {handler = nullptr; }

  srs_cu_up::ngu_tnl_pdu_session* handler;
};

class gtpu_rx_m1_adapter : public srsran::gtpu_tunnel_mru_rx_lower_layer_notifier
{
public:
  /// \brief Interface for the GTP-U to pass a SDU (i.e. MR-U DL message) into the lower layer.
  /// \param dl_message MR-U DL message with optional T-PDU.
  void on_new_sdu(mru_dl_message dl_message) override
  {
    if (handler != nullptr) {
      handler->on_new_pdu(std::move(dl_message));
    }
  }

  /// \brief Interface for the GTP-U to pass a SDU (i.e. MR-U UL message) into the lower layer.
  /// \param ul_message MR-U UL message with optional T-PDU
  void on_new_sdu(mru_ul_buffer_state ul_message) override {}

  void connect(m1_du_gateway_bearer_rx_notifier& handler_) { handler = &handler_; }

  void disconnect() { handler = nullptr; }

  m1_du_gateway_bearer_rx_notifier* handler;
};

/// Adapter between Network Gateway (Data) and GTP-U demux
class network_gateway_data_gtpu_demux_adapter : public srsran::network_gateway_data_notifier_with_src_addr
{
public:
  network_gateway_data_gtpu_demux_adapter()           = default;
  ~network_gateway_data_gtpu_demux_adapter() override = default;

  void connect_gtpu_demux(gtpu_demux_rx_upper_layer_interface& gtpu_demux_) { gtpu_demux = &gtpu_demux_; }

  void on_new_pdu(byte_buffer pdu, const sockaddr_storage& src_addr) override
  {
    srsran_assert(gtpu_demux != nullptr, "GTP-U handler must not be nullptr");
    gtpu_demux->handle_pdu(std::move(pdu), src_addr);
  }

private:
  gtpu_demux_rx_upper_layer_interface* gtpu_demux = nullptr;
};

/// \brief Object used to represent a bearer at the CU M1 gateway
/// On the co-located case this is done by connecting both entities directly.
///
/// It will keep a notifier to the DU MR-U RX and provide the methods to pass
/// an SDU to it.
class m1_split_gateway_du_bearer : public m1_du_gateway_bearer
{
public:
  m1_split_gateway_du_bearer(uint32_t                                  ue_index,
                             drb_id_t                                  drb_id,
                             const up_transport_layer_info&            dl_tnl_info_,
                             srs_du::m1_du_gateway_bearer_rx_notifier& du_rx_,
                             const up_transport_layer_info&            ul_up_tnl_info_,
                             srs_du::m1_bearer_disconnector&           disconnector_,
                             dlt_pcap&                                 gtpu_pcap,
                             uint16_t                                  peer_port) :
    logger("DU-M1", {ue_index, drb_id, dl_tnl_info_}),
    disconnector(disconnector_),
    dl_tnl_info(dl_tnl_info_),
    ul_tnl_info(ul_up_tnl_info_),
    du_rx(du_rx_)
  {
    gtpu_to_m1_adapter.connect(du_rx);

    gtpu_tunnel_mru_creation_message msg{};
    // msg.ue_index = 0; // TODO
    msg.cfg.rx.local_teid = dl_tnl_info.gtp_teid;
    msg.cfg.tx.peer_teid  = ul_tnl_info.gtp_teid;
    msg.cfg.tx.peer_addr  = ul_tnl_info.tp_address.to_string();
    msg.cfg.tx.peer_port  = peer_port;
    msg.gtpu_pcap         = &gtpu_pcap;
    msg.tx_upper          = &gtpu_to_network_adapter;
    msg.rx_lower          = &gtpu_to_m1_adapter;

    tunnel = srsran::create_gtpu_tunnel_mru(msg);
  }

  ~m1_split_gateway_du_bearer() override { stop(); }

  void stop() override { disconnector.remove_du_bearer(dl_tnl_info); }

  void on_new_pdu(mru_ul_buffer_state msg) override
  {
    if (tunnel == nullptr) {
      logger.log_debug("DL GTPU tunnel not connected. Discarding SDU.");
      return;
    }
    tunnel->get_tx_lower_layer_interface()->handle_sdu(std::move(msg));
  }

  gtpu_tunnel_common_rx_upper_layer_interface* get_tunnel_rx_interface()
  {
    return tunnel->get_rx_upper_layer_interface();
  }

  /// Holds the RX executor associated with the M1 bearer.
  // task_executor& dl_exec;

  gtpu_tx_udp_gw_adapter gtpu_to_network_adapter;
  gtpu_rx_m1_adapter     gtpu_to_m1_adapter;

private:
  m1_bearer_logger                 logger;
  m1_bearer_disconnector&          disconnector;
  up_transport_layer_info          dl_tnl_info;
  up_transport_layer_info          ul_tnl_info;
  std::unique_ptr<gtpu_tunnel_mru> tunnel;

public:
  /// Holds notifier that will point to MR-U bearer on the DL path
  m1_du_gateway_bearer_rx_notifier& du_rx;
};

/// \brief Object used to connect the DU and CU-UP F1-U bearers
/// On the co-located case this is done by connecting both entities directly.
///
/// Note that CU and DU bearer creation and removal can be performed from different threads and are therefore
/// protected by a common mutex.
class m1_split_connector final : public m1_du_udp_gateway
{
public:
  m1_split_connector(srs_cu_up::ngu_gateway*  udp_gw_,
                      gtpu_demux*             demux_,
                      dlt_pcap&               gtpu_pcap_,
                      uint16_t                peer_port_    = GTPU_PORT,
                      std::string             m1_ext_addr_ = "auto") :
    logger_du(srslog::fetch_basic_logger("DU-M1")),
    udp_gw(udp_gw_),
    demux(demux_),
    gtpu_pcap(gtpu_pcap_),
    peer_port(peer_port_),
    m1_ext_addr(std::move(m1_ext_addr_))
  {
    udp_session = udp_gw->create(gw_data_gtpu_demux_adapter);
    gw_data_gtpu_demux_adapter.connect_gtpu_demux(*demux);
  }

  m1_du_gateway* get_m1_du_gateway() { return this; }

  std::optional<uint16_t> get_bind_port() const override { return udp_session->get_bind_port(); }

  std::unique_ptr<m1_du_gateway_bearer> create_du_bearer(uint32_t                                  ue_index,
                                                         drb_id_t                                  drb_id,
                                                         srs_du::m1_config                         config,
                                                         const up_transport_layer_info&            dl_up_tnl_info,
                                                         const up_transport_layer_info&            ul_up_tnl_info,
                                                         srs_du::m1_du_gateway_bearer_rx_notifier& du_rx,
                                                         timer_factory                             timers,
                                                         task_executor& ue_executor) override;

  void remove_du_bearer(const up_transport_layer_info& dl_up_tnl_info) override;

  expected<std::string> get_du_bind_address(gnb_du_id_t gnb_du_id) const override;

private:
  srslog::basic_logger& logger_du;
  // Key is the UL UP TNL Info (CU-CP address and UL TEID reserved by CU-CP)
  std::unordered_map<up_transport_layer_info, m1_split_gateway_du_bearer*> du_map;
  std::mutex map_mutex; // shared mutex for access to cu_map

  srs_cu_up::ngu_gateway*                         udp_gw;
  std::unique_ptr<srs_cu_up::ngu_tnl_pdu_session> udp_session;
  gtpu_demux*                                     demux;
  network_gateway_data_gtpu_demux_adapter         gw_data_gtpu_demux_adapter;
  dlt_pcap&                                       gtpu_pcap;

  uint16_t    peer_port;
  std::string m1_ext_addr = "auto"; // External address advertised by the M1 interface
};

} // namespace srsran::srs_du