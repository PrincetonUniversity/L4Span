/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "du_bearer.h"
#include "../converters/rlc_config_helpers.h"
#include "srsran/du/du_high/du_manager/du_manager_params.h"
#include "srsran/f1u/du/f1u_bearer_factory.h"
#include "srsran/m1/du/m1_bearer_factory.h"
#include "srsran/gtpu/gtpu_teid_pool.h"
#include "srsran/srslog/srslog.h"

using namespace srsran;
using namespace srs_du;

void du_srb_connector::connect(du_ue_index_t                       ue_index,
                               srb_id_t                            srb_id,
                               f1c_bearer&                         f1_bearer,
                               rlc_entity&                         rlc_bearer,
                               mac_ue_control_information_handler& mac_ue_info_handler)
{
  // > Connect RLC Rx SDU -> F1-C Tx SDU handler.
  rlc_rx_sdu_notif.connect(f1_bearer);

  // > Connect RLC Tx Data Notifier -> F1-C handler
  rlc_tx_data_notif.connect(f1_bearer);

  // > Connect F1-C Rx SDU -> RLC Tx SDU.
  f1c_rx_sdu_notif.connect(*rlc_bearer.get_tx_upper_layer_data_interface());

  // > Connect RLC BSR update notifier -> MAC Control Info Handler.
  rlc_tx_buffer_state_notif.connect(ue_index, srb_id_to_lcid(srb_id), mac_ue_info_handler);

  // > Connect MAC Rx SDU notifier -> RLC Rx PDU.
  mac_rx_sdu_notifier.connect(*rlc_bearer.get_rx_lower_layer_interface());

  // > Connect MAC Tx SDU builder -> RLC Tx PDU builder.
  mac_tx_sdu_notifier.connect(*rlc_bearer.get_tx_lower_layer_interface());
}

void du_srb_connector::disconnect()
{
  // Disconnect F1AP <-> RLC interface.
  f1c_rx_sdu_notif.disconnect();
  rlc_tx_data_notif.disconnect();
  rlc_rx_sdu_notif.disconnect();

  // Disconnect MAC <-> RLC interface.
  mac_rx_sdu_notifier.disconnect();
  rlc_tx_buffer_state_notif.disconnect();
}

void du_ue_srb::stop()
{
  connector.disconnect();
  rlc_bearer->stop();
}

void du_drb_connector::connect(du_ue_index_t                       ue_index,
                               drb_id_t                            drb_id,
                               lcid_t                              lcid,
                               f1u_tx_pdu_notifier&                f1_gw_bearer,
                               f1u_bearer&                         f1_bearer,
                              //  m1_tx_pdu_notifier&                 m1_gw_bearer,
                              //  m1_bearer&                          m1_bearer,
                               rlc_entity&                         rlc_bearer,
                               mac_ue_control_information_handler& mac_ue_info_handler)
{
  // > Connect F1-U Rx SDU -> F1-U GW Tx SDU handler.
  f1u_gateway_nru_rx_notif.connect(f1_bearer.get_rx_pdu_handler());

  // // > Connect M1 Rx SDU -> M1 GW Tx SDU handler. For DL M1 data.
  // m1_gateway_mru_rx_notif.connect(m1_bearer.get_rx_pdu_handler());

  // > Connect RLC Rx SDU -> F1-U Tx SDU handler.
  rlc_rx_sdu_notif.connect(f1_bearer.get_tx_sdu_handler());

  // // > Connect RLC Rx SDU -> M1 Tx SDU handler.
  // rlc_tx_m1_sdu_notif.connect(m1_bearer.get_tx_sdu_handler());

  // > Connect RLC Tx Data Notifier -> F1-U handler
  rlc_tx_data_notif.connect(f1_bearer.get_tx_delivery_handler());

  // > Connect F1-U Rx SDU -> RLC Tx SDU.
  f1u_rx_sdu_notif.connect(*rlc_bearer.get_tx_upper_layer_data_interface());

  // // > Connect M1 Rx SDU -> RLC Tx SDU. 
  // // TODO_HW: Maynot be necessary, to be verified in the future. 
  // m1_rx_sdu_notif.connect(*rlc_bearer.get_tx_m1_data_interface());

  // > Connect RLC BSR update notifier -> MAC Control Info Handler.
  rlc_tx_buffer_state_notif.connect(ue_index, lcid, mac_ue_info_handler);

  // > Connect MAC Rx SDU notifier -> RLC Rx PDU.
  mac_rx_sdu_notifier.connect(*rlc_bearer.get_rx_lower_layer_interface());

  // > Connect MAC Tx SDU builder -> RLC Tx PDU builder.
  mac_tx_sdu_notifier.connect(*rlc_bearer.get_tx_lower_layer_interface());
}

void du_drb_connector::disconnect()
{
  // Disconnect NR-U <-> F1-U GW interface
  f1u_gateway_nru_rx_notif.disconnect();

  // Disconnect NR-U <-> RLC interface.
  rlc_rx_sdu_notif.disconnect();
  rlc_tx_data_notif.disconnect();
  f1u_rx_sdu_notif.disconnect();

  // rlc_tx_m1_sdu_notif.disconnect();
  // m1_rx_sdu_notif.disconnect();

  // Disconnect MAC <-> RLC interface.
  mac_rx_sdu_notifier.disconnect();
  rlc_tx_buffer_state_notif.disconnect();
  mac_tx_sdu_notifier.disconnect();
}

void du_ue_drb::stop()
{
  connector.disconnect();
  rlc_bearer->stop();
  drb_f1u->stop();
  // drb_m1->stop();
}

std::unique_ptr<du_ue_drb> srsran::srs_du::create_drb(const drb_creation_info& drb_info)
{
  srsran_assert(not is_srb(drb_info.lcid), "Invalid DRB LCID={}", drb_info.lcid);
  // srsran_assert(not drb_info.uluptnl_info_list.empty(), "Invalid UP TNL Info list");

  const du_ue_index_t ue_index  = drb_info.ue_index;
  f1u_du_gateway&     f1u_gw    = drb_info.du_params.f1u.f1u_gw;
  // m1_du_gateway&      m1_gw     = drb_info.du_params.m1.m1_gw;
  gtpu_teid_pool&     teid_pool = drb_info.teid_pool;

  // > Get DL UP TNL bind address.
  expected<std::string> f1u_bind_string = f1u_gw.get_du_bind_address(drb_info.du_params.ran.gnb_du_id);
  srsran_assert(f1u_bind_string.has_value(), "Unable to bind F1U address");
  transport_layer_address f1u_bind_addr = transport_layer_address::create_from_string(f1u_bind_string.value());

  // // > Get DL M1 TNL bind address, 
  // // I dont think this is necessary. But we can config it and use it in the future.
  // expected<std::string> m1_bind_string = m1_gw.get_du_bind_address(drb_info.du_params.ran.gnb_du_id);
  // srsran_assert(m1_bind_string.has_value(), "Unable to bind M1 address");
  // transport_layer_address m1_bind_addr = transport_layer_address::create_from_string(m1_bind_string.value());

  srslog::fetch_basic_logger("DU-MNG").debug("Binding M1 address.");


  // > Setup DL UP TNL info.
  expected<gtpu_teid_t> dl_teid = teid_pool.request_teid();
  if (not dl_teid.has_value()) {
    srslog::fetch_basic_logger("DU-MNG").warning("ue={}: Failed to allocate DL GTP-TEID.", ue_index);
    return nullptr;
  }
  // Note: We are computing the DL GTP-TEID as a concatenation of the UE index and DRB-id.
  std::array<up_transport_layer_info, 1> dluptnl_info_list = {up_transport_layer_info{f1u_bind_addr, dl_teid.value()}};

  // // M1 interface use the same teid as the F1-U interface for convenience.
  // std::array<up_transport_layer_info, 1> dlupm1_info_list = {up_transport_layer_info{m1_bind_addr, dl_teid.value()}};

  // > Create DRB instance.
  std::unique_ptr<du_ue_drb> drb = std::make_unique<du_ue_drb>();

  // > Setup DRB config
  drb->drb_id = drb_info.drb_id;
  drb->lcid   = drb_info.lcid;
  drb->uluptnl_info_list.assign(drb_info.uluptnl_info_list.begin(), drb_info.uluptnl_info_list.end());
  drb->dluptnl_info_list.assign(dluptnl_info_list.begin(), dluptnl_info_list.end());
  // drb->ulupm1_info_list.assign(drb_info.ulupm1_info_list.begin(), drb_info.ulupm1_info_list.end());
  // drb->dlupm1_info_list.assign(dlupm1_info_list.begin(), dlupm1_info_list.end());
  // srslog::fetch_basic_logger("DU-MNG").debug("m1 ul address {}.", drb_info.ulupm1_info_list[0].tp_address.to_string());

  drb->f1u_gw_bearer = drb_info.du_params.f1u.f1u_gw.create_du_bearer(
      ue_index,
      drb->drb_id,
      drb_info.f1u_cfg,
      drb->dluptnl_info_list[0],
      drb->uluptnl_info_list[0],
      drb->connector.f1u_gateway_nru_rx_notif,
      timer_factory{drb_info.du_params.services.timers, drb_info.du_params.services.ue_execs.ctrl_executor(ue_index)},
      drb_info.du_params.services.ue_execs.f1u_dl_pdu_executor(ue_index));
  if (drb->f1u_gw_bearer == nullptr) {
    srslog::fetch_basic_logger("DU-MNG").warning("ue={}: Failed to connect F1-U GW bearer to CU-UP.", ue_index);
    return nullptr;
  }

  // > Create F1-U bearer.
  f1u_bearer_creation_message f1u_msg = {};
  f1u_msg.ue_index                    = ue_index;
  f1u_msg.drb_id                      = drb->drb_id;
  f1u_msg.config                      = drb_info.f1u_cfg;
  f1u_msg.dl_tnl_info                 = drb->dluptnl_info_list[0];
  f1u_msg.rx_sdu_notifier             = &drb->connector.f1u_rx_sdu_notif;
  f1u_msg.tx_pdu_notifier             = drb->f1u_gw_bearer.get();
  f1u_msg.timers =
      timer_factory{drb_info.du_params.services.timers, drb_info.du_params.services.ue_execs.ctrl_executor(ue_index)};
  f1u_msg.ue_executor  = &drb_info.du_params.services.ue_execs.f1u_dl_pdu_executor(ue_index);
  f1u_msg.disconnector = &drb_info.du_params.f1u.f1u_gw;

  drb->drb_f1u = srs_du::create_f1u_bearer(f1u_msg);
  if (drb->f1u_gw_bearer == nullptr) {
    srslog::fetch_basic_logger("DU-MNG").warning("ue={}: Failed to create F1-U bearer.", ue_index);
    return nullptr;
  }

  // drb->m1_gw_bearer = drb_info.du_params.m1.m1_gw.create_du_bearer(
  //     ue_index,
  //     drb->drb_id,
  //     drb_info.m1_cfg,
  //     drb->dlupm1_info_list[0],
  //     drb->ulupm1_info_list[0],
  //     drb->connector.m1_gateway_mru_rx_notif,
  //     timer_factory{drb_info.du_params.services.timers, drb_info.du_params.services.ue_execs.ctrl_executor(ue_index)},
  //     drb_info.du_params.services.ue_execs.m1_ul_pdu_executor(ue_index)); 
  // // TODO_hw: Verify if we need a new executor, as there will be no dl message
  // if (drb->m1_gw_bearer == nullptr) {
  //   srslog::fetch_basic_logger("DU-MNG").warning("ue={}: Failed to connect M1 GW bearer to CU-UP.", ue_index);
  //   return nullptr;
  // }

  // // > Create F1-U bearer.
  // m1_bearer_creation_message m1_msg = {};
  // m1_msg.ue_index                    = ue_index;
  // m1_msg.drb_id                      = drb->drb_id;
  // m1_msg.config                      = drb_info.m1_cfg;
  // m1_msg.dl_tnl_info                 = drb->dlupm1_info_list[0];
  // m1_msg.rx_sdu_notifier             = &drb->connector.m1_rx_sdu_notif;
  // m1_msg.tx_pdu_notifier             = drb->m1_gw_bearer.get();
  // // TODO_hw: Do we need a timer?
  // m1_msg.timers =
  //     timer_factory{drb_info.du_params.services.timers, drb_info.du_params.services.ue_execs.ctrl_executor(ue_index)};
  // // TODO_hw: Verify if we need a new executor, as there will be no dl message
  // m1_msg.ue_executor  = &drb_info.du_params.services.ue_execs.m1_ul_pdu_executor(ue_index);
  // m1_msg.disconnector = &drb_info.du_params.m1.m1_gw;

  // drb->drb_m1 = srs_du::create_m1_bearer(m1_msg);
  // if (drb->m1_gw_bearer == nullptr) {
  //   srslog::fetch_basic_logger("DU-MNG").warning("ue={}: Failed to create M1 bearer.", ue_index);
  //   return nullptr;
  // }

  // > Create RLC DRB entity.
  drb->rlc_bearer = create_rlc_entity(make_rlc_entity_creation_message(drb_info.du_params.ran.gnb_du_id,
                                                                       ue_index,
                                                                       drb_info.pcell_index,
                                                                       *drb,
                                                                       drb_info.rlc_cfg,
                                                                       drb_info.du_params.services,
                                                                       drb_info.rlc_rlf_notifier,
                                                                       drb_info.du_params.rlc.rlc_metrics_notif,
                                                                       drb_info.du_params.rlc.pcap_writer));
  if (drb->rlc_bearer == nullptr) {
    // Failed to create RLC DRB entity.
    f1u_gw.remove_du_bearer(drb->dluptnl_info_list[0]);
    return nullptr;
  }

  // > Connect DRB F1-U with RLC, and RLC with MAC logical channel notifier.
  // > HW: Connect DRB M1 with RLC for the UL feedback.
  drb->connector.connect(ue_index,
                         drb->drb_id,
                         drb->lcid,
                         *drb->f1u_gw_bearer,
                         *drb->drb_f1u,
                        //  *drb->m1_gw_bearer,
                        //  *drb->drb_m1,
                         *drb->rlc_bearer,
                         drb_info.du_params.rlc.mac_ue_info_handler);

  return drb;
}
