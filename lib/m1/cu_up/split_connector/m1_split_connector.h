#pragma once 

#include "srsran/m1/cu_up/m1_bearer_logger.h"
#include "srsran/m1/cu_up/m1_gateway.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/gtpu_tunnel_common_rx.h"
#include "srsran/gtpu/gtpu_tunnel_mru.h"
#include "srsran/gtpu/gtpu_tunnel_mru_rx.h"
#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/srslog/srslog.h"
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace srsran::srs_cu_up {

class gtpu_tx_udp_gw_adapter;
class gtpu_rx_m1_adapter;
class network_gateway_data_gtpu_demux_adapter;

/// \brief Object used to represent a bearer at the CU M1 gateway
/// On the co-located case this is done by connecting both entities directly.
///
/// It will keep a notifier to the DU MR-U RX and provide the methods to pass
/// an SDU to it.
class m1_split_gateway_cu_bearer final : public m1_cu_up_gateway_bearer
{
public:
  m1_split_gateway_cu_bearer(uint32_t                             ue_index_,
                             drb_id_t                             drb_id,
                             const up_transport_layer_info&       ul_tnl_info_,
                             m1_cu_up_gateway_bearer_rx_notifier& cu_rx_,
                             ngu_tnl_pdu_session&                 udp_session,
                             task_executor&                       ul_exec_,
                             srs_cu_up::m1_bearer_disconnector&   disconnector_);

  ~m1_split_gateway_cu_bearer() override;

  void stop() override;

  void on_new_pdu(mru_dl_message msg) override
  {
    if (tunnel_tx == nullptr) {
      logger.log_debug("DL GTPU tunnel not connected. Discarding SDU.");
      return;
    }
    tunnel_tx->handle_sdu(std::move(msg));
  }

  void attach_tunnel_rx(std::unique_ptr<gtpu_tunnel_common_rx_upper_layer_interface> tunnel_rx_)
  {
    tunnel_rx = std::move(tunnel_rx_);
  }

  void attach_tunnel_tx(std::unique_ptr<gtpu_tunnel_mru_tx_lower_layer_interface> tunnel_tx_)
  {
    tunnel_tx = std::move(tunnel_tx_);
  }

  std::unique_ptr<gtpu_tx_udp_gw_adapter> gtpu_to_network_adapter;
  std::unique_ptr<gtpu_rx_m1_adapter> gtpu_to_m1_adapter;

  gtpu_tunnel_common_rx_upper_layer_interface* get_tunnel_rx_interface() { return tunnel_rx.get(); }

  /// Holds the RX executor associated with the F1-U bearer.
  task_executor& ul_exec;
  uint32_t       ue_index;

private:
  bool                                                         stopped = false;
  srs_cu_up::m1_bearer_logger                                  logger;
  srs_cu_up::m1_bearer_disconnector&                           disconnector;
  up_transport_layer_info                                      ul_tnl_info;
  std::unique_ptr<gtpu_tunnel_common_rx_upper_layer_interface> tunnel_rx;
  std::unique_ptr<gtpu_tunnel_mru_tx_lower_layer_interface>    tunnel_tx;

public:
  /// Holds notifier that will point to MR-U bearer on the UL path
  m1_cu_up_gateway_bearer_rx_notifier& cu_rx;

  /// Holds the DL UP TNL info associated with the M1 bearer
  std::optional<up_transport_layer_info> dl_tnl_info;
};

/// \brief Object used to connect the DU and CU-UP M1 bearers
/// On the co-located case this is done by connecting both entities directly.
///
/// Note that CU and DU bearer creation and removal can be performed from different threads and are therefore
/// protected by a common mutex.
class m1_split_connector final : public m1_cu_up_udp_gateway
{
public:
  m1_split_connector(ngu_gateway& udp_gw_, gtpu_demux& demux_, dlt_pcap& gtpu_pcap_, uint16_t peer_port_ = M1_GTPU_PORT);
  ~m1_split_connector() override;

  m1_cu_up_gateway* get_m1_cu_up_gateway() { return this; }

  std::optional<uint16_t> get_bind_port() const override { return udp_session->get_bind_port(); }

  std::unique_ptr<m1_cu_up_gateway_bearer> create_cu_bearer(uint32_t                             ue_index,
                                                            drb_id_t                             drb_id,
                                                            const srs_cu_up::m1_config&          config,
                                                            const up_transport_layer_info&       ul_up_tnl_info,
                                                            m1_cu_up_gateway_bearer_rx_notifier& rx_notifier,
                                                            task_executor&                       ul_exec) override;
  
  void attach_dl_teid(const up_transport_layer_info& ul_up_tnl_info,
                      const up_transport_layer_info& dl_up_tnl_info) override;

  void disconnect_cu_bearer(const up_transport_layer_info& ul_up_tnl_info) override;

  expected<std::string> get_cu_bind_address() const override;

private:
  srslog::basic_logger& logger_cu;
  // Key is the UL UP TNL Info (CU-CP address and UL TEID reserved by CU-CP)
  std::unordered_map<up_transport_layer_info, m1_split_gateway_cu_bearer*> cu_map;
  std::mutex map_mutex; // shared mutex for access to cu_map

  uint16_t                                                 peer_port;
  ngu_gateway&                                             udp_gw;
  std::unique_ptr<ngu_tnl_pdu_session>                     udp_session;
  gtpu_demux&                                              demux;
  std::unique_ptr<network_gateway_data_gtpu_demux_adapter> gw_data_gtpu_demux_adapter;
  dlt_pcap&                                                gtpu_pcap;
};

} // namespace srsran::srs_cu_up