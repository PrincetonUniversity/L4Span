#pragma once

#include "srsran/m1/cu_up/m1_bearer.h"
#include "srsran/m1/cu_up/m1_config.h"
#include "srsran/m1/cu_up/m1_tx_pdu_notifier.h"
#include "srsran/ran/lcid.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

/// This class provides a notifier for the RX bearer
/// inside the CU-UP M1-U gateway. This provides an adapter
/// to the MR-U bearer to pass SDUs into.
class m1_cu_up_gateway_bearer_rx_notifier
{
public:
  virtual ~m1_cu_up_gateway_bearer_rx_notifier() = default;

  virtual void on_new_pdu(mru_ul_buffer_state msg) = 0;
};

/// This class provides the interface for an M1 GW bearer
class m1_cu_up_gateway_bearer : public srs_cu_up::m1_tx_pdu_notifier
{
public:
  virtual void stop() = 0;
};

/// This class will be used to provide the interfaces to
/// the CU-UP to create and manage M1 bearers.
class m1_cu_up_gateway : public srs_cu_up::m1_bearer_disconnector
{
public:
  m1_cu_up_gateway()                                    = default;
  ~m1_cu_up_gateway() override                          = default;
  m1_cu_up_gateway(const m1_cu_up_gateway&)             = default;
  m1_cu_up_gateway& operator=(const m1_cu_up_gateway&)  = default;
  m1_cu_up_gateway(m1_cu_up_gateway&&)                  = default;
  m1_cu_up_gateway& operator=(m1_cu_up_gateway&&)       = default;

  virtual std::unique_ptr<m1_cu_up_gateway_bearer> create_cu_bearer(uint32_t                        ue_index,
                                                                     drb_id_t                       drb_id,
                                                                     const srs_cu_up::m1_config&    config,
                                                                     const up_transport_layer_info& ul_up_tnl_info,
                                                                     m1_cu_up_gateway_bearer_rx_notifier& rx_notifier,
                                                                     task_executor&                       ul_exec) = 0;

  virtual void attach_dl_teid(const up_transport_layer_info& ul_up_tnl_info,
                              const up_transport_layer_info& dl_up_tnl_info) = 0;

  virtual expected<std::string> get_cu_bind_address() const = 0;
};

/// This class will be used to provide the interfaces to
/// the CU-UP to create and manage M1 bearers.
class m1_cu_up_udp_gateway : public m1_cu_up_gateway
{
public:
  m1_cu_up_udp_gateway()                                        = default;
  ~m1_cu_up_udp_gateway() override                              = default;
  m1_cu_up_udp_gateway(const m1_cu_up_udp_gateway&)             = default;
  m1_cu_up_udp_gateway& operator=(const m1_cu_up_udp_gateway&)  = default;
  m1_cu_up_udp_gateway(m1_cu_up_udp_gateway&&)                  = default;
  m1_cu_up_udp_gateway& operator=(m1_cu_up_udp_gateway&&)       = default;

  virtual std::optional<uint16_t> get_bind_port() const = 0;
};

} // namespace srsran