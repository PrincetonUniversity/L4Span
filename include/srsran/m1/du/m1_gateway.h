#pragma once

#include "srsran/m1/du/m1_bearer.h"
#include "srsran/m1/du/m1_config.h"
#include "srsran/m1/du/m1_tx_pdu_notifier.h"
#include "srsran/ran/gnb_du_id.h"
#include "srsran/ran/lcid.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/timers.h"

namespace srsran::srs_du {

/// This class provides a notifier for the RX bearer
/// inside the DU M1 gateway. This provides an adapter
/// to the NR-U bearer to pass SDUs into.
class m1_du_gateway_bearer_rx_notifier
{
public:
  virtual ~m1_du_gateway_bearer_rx_notifier() = default;

  virtual void on_new_pdu(mru_dl_message msg) = 0;
};

/// This class provides the interface for an F1-U GW bearer
class m1_du_gateway_bearer : public srs_du::m1_tx_pdu_notifier
{
  virtual void stop() = 0;
};

/// This class will be used to provide the interfaces to
/// the DU to create and manage F1-U bearers.
class m1_du_gateway : public srs_du::m1_bearer_disconnector
{
public:
  m1_du_gateway()                                 = default;
  ~m1_du_gateway() override                       = default;
  m1_du_gateway(const m1_du_gateway&)            = default;
  m1_du_gateway& operator=(const m1_du_gateway&) = default;
  m1_du_gateway(m1_du_gateway&&)                 = default;
  m1_du_gateway& operator=(m1_du_gateway&&)      = default;

  virtual std::unique_ptr<m1_du_gateway_bearer> create_du_bearer(uint32_t                       ue_index,
                                                                  drb_id_t                       drb_id,
                                                                  srs_du::m1_config             config,
                                                                  const up_transport_layer_info& dl_up_tnl_info,
                                                                  const up_transport_layer_info& ul_up_tnl_info,
                                                                  srs_du::m1_du_gateway_bearer_rx_notifier& du_rx,
                                                                  timer_factory                              timers,
                                                                  task_executor& ue_executor) = 0;

  virtual expected<std::string> get_du_bind_address(gnb_du_id_t gnb_du_id) const = 0;
};

/// This class will be used to provide the interfaces to
/// the CU-UP to create and manage F1-U bearers.
class m1_du_udp_gateway : public m1_du_gateway
{
public:
  m1_du_udp_gateway()                                     = default;
  ~m1_du_udp_gateway() override                           = default;
  m1_du_udp_gateway(const m1_du_udp_gateway&)            = default;
  m1_du_udp_gateway& operator=(const m1_du_udp_gateway&) = default;
  m1_du_udp_gateway(m1_du_udp_gateway&&)                 = default;
  m1_du_udp_gateway& operator=(m1_du_udp_gateway&&)      = default;

  virtual std::optional<uint16_t> get_bind_port() const = 0;
};

} // namespace srsran::srs_du
