#pragma once

#include "mark_entity_rx_impl.h"
#include "mark_entity_tx_impl.h"
#include "mark_session_logger.h"
#include "srsran/mark/mark.h"
#include "srsran/support/timers.h"
#include <unordered_map>
#include "srsran/ctsa/ctsa.h"

namespace srsran {

namespace srs_cu_up {

class mark_entity_impl : public mark_entity, 
                         public mark_tx_sdu_handler,
                        //  public mark_rx_m1_interface,
                        //  public mark_tx_m1_interface
                         public mark_tx_lower_interface,
                         public mark_rx_lower_interface
{
public:
  mark_entity_impl(uint32_t ue_index_, 
                   pdu_session_id_t psi_, 
                   mark_rx_sdu_notifier& rx_sdu_notifier_,
                   uint32_t nof_drbs) :
    logger("MARK", {ue_index_, psi_}), 
    ue_index(ue_index_), 
    psi(psi_), 
    rx_sdu_notifier(rx_sdu_notifier_)
  {
    dequeue_rate_cal_wind = 50;
    dequeue_rate_pred_wind = 50;
    l4s_tq_thr = 10000; // 10000 ns = 10 ms;
    classic_tq_thr = 100000; // 50000 ns = 50 ms;
    dequeue_history = (double*)malloc(sizeof(double) * dequeue_rate_pred_wind);
    nof_ue = 1;
  }
  ~mark_entity_impl() override = default;

  mark_rx_pdu_handler& get_mark_rx_pdu_handler() final{ return *rx.get(); };
  mark_tx_sdu_handler& get_mark_tx_sdu_handler() final { return *this; };
  // mark_rx_m1_interface& get_mark_rx_m1_interface() final { return *this; };
  // mark_tx_m1_interface& get_mark_tx_m1_interface() final { return *this; };
  mark_rx_lower_interface& get_mark_rx_lower_interface() final { return *this; };
  mark_tx_lower_interface& get_mark_tx_lower_interface() final { return *this; };

  /// Handle the incoming SDU and redirect to mapped DRB.
  void handle_sdu(byte_buffer sdu, qos_flow_id_t qos_flow_id) final
  {
    logger.log_info("TX PDU. pdu_len={}, qfi={}", sdu.length(), qos_flow_id);
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto ts = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    auto sdu_it = sdu.segments().begin();
    std::stringstream tmp_string;
    tmp_string << std::hex << std::setfill('0');
    while (sdu_it != sdu.segments().end()) 
    {
      // for (unsigned int buffer_i = 0; buffer_i < (*sdu_it).size(); buffer_i++) {
      //   tmp_string << std::hex << std::setw(2) << static_cast<int>((*sdu_it).data()[buffer_i]);
      // }
      drb_id_t drb_id = qfi_to_drb[qos_flow_id];
      iphdr* ipv4_hdr = (iphdr*)malloc(sizeof(iphdr));
      memcpy(ipv4_hdr, (*sdu_it).data(), sizeof(iphdr));
      ip::swap_iphdr(ipv4_hdr);

      ip::five_tuple pkt_five_tuple = {};

      // logger.log_debug("IP header version {}, ihl {}, tos {}, total length {},"
      //                  " id {}, protocol {}, src_addr {}, dst_addr {}", 
      //                  (uint8_t)ipv4_hdr->version,
      //                  (uint8_t)ipv4_hdr->ihl,
      //                  (uint8_t)ipv4_hdr->tos,
      //                  ipv4_hdr->tot_len,
      //                  ipv4_hdr->id,
      //                  (uint8_t)ipv4_hdr->protocol,
      //                  ipv4_hdr->saddr,
      //                  ipv4_hdr->daddr);

      if (ipv4_hdr->protocol == 6) {
        tcphdr* tcp_hdr = (tcphdr*)malloc(sizeof(tcphdr));
        memcpy(tcp_hdr, (*sdu_it).data()+sizeof(iphdr), sizeof(tcphdr));
        ip::swap_tcphdr(tcp_hdr);

        pkt_five_tuple = ip::extract_five_tuple(*ipv4_hdr, *tcp_hdr);
        // logger.log_debug("TCP header source {}, dst {}, seq {}, ack seq {}, "
        //                  "offset {}, cwr {}, ece {}, urg {}, ack {}, psh {}, "
        //                  "rst {}, syn {}, fin {}, window {}, checksum {}, urg_ptr {}", 
        //                  tcp_hdr->source,
        //                  tcp_hdr->dest,
        //                  tcp_hdr->seq,
        //                  tcp_hdr->ack_seq,
        //                  (uint8_t)tcp_hdr->doff,
        //                  (uint8_t)tcp_hdr->cwr,
        //                  (uint8_t)tcp_hdr->ece,
        //                  (uint8_t)tcp_hdr->urg,
        //                  (uint8_t)tcp_hdr->ack,
        //                  (uint8_t)tcp_hdr->psh,
        //                  (uint8_t)tcp_hdr->rst,
        //                  (uint8_t)tcp_hdr->syn,
        //                  (uint8_t)tcp_hdr->fin,
        //                  tcp_hdr->window,
        //                  tcp_hdr->check,
        //                  tcp_hdr->urg_ptr); 

        // Flow mapping: map 5 tuples of IP/TCP flow to DRB id.
        rx.get()->five_tuple_to_drb[pkt_five_tuple].drb_id = drb_id;

        if (!(uint8_t)tcp_hdr->syn){
          if (rx.get()->five_tuple_to_rtt[pkt_five_tuple].ingress_of_second == 0 && 
              rx.get()->five_tuple_to_rtt[pkt_five_tuple].ingress_of_syn != 0) {
            rx.get()->five_tuple_to_rtt[pkt_five_tuple].estimated_rtt = 
              rx.get()->five_tuple_to_rtt[pkt_five_tuple].ingress_of_second - 
              rx.get()->five_tuple_to_rtt[pkt_five_tuple].ingress_of_syn;
          }
          // Skip the SYN == 1 packet
          uint8_t ect = ipv4_hdr->tos & ip::INET_ECN_MASK;
          if (ect == ip::INET_ECN_ECT_1){
            // L4S, perform mark
            // logger.log_debug("L4S flow, drb {}, marking prob {}", drb_id, rx.get()->drb_flow_state[drb_id].mark_l4s);
            if (rand() < rx.get()->drb_flow_state[drb_id].mark_l4s) {

              // Do the downlink packet marking
              // rx.get()->perform_ip_mark((*sdu_it).data(), ipv4_hdr, drb_id, pkt_five_tuple);

              // Save the marking information and mark the uplink ACK instead
              rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
              rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4; 

              // rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn1 += 1;
              // rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn1 += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4;

            } else {
              // Update the packet flow information
              rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn1 += 1;
              rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn1 += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4;
            }
          } else if (ect == ip::INET_ECN_ECT_0){
            
            // logger.log_debug("Classic flow, drb {}, marking prob {}", drb_id, rx.get()->drb_flow_state[drb_id].mark_classic);
            if (rand() < rx.get()->drb_flow_state[drb_id].mark_classic) {
              // Do the downlink packet marking
              // rx.get()->perform_ip_mark((*sdu_it).data(), ipv4_hdr, drb_id, pkt_five_tuple);
              
              // Save the marking information and mark the uplink ACK instead
              rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
              rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4; 

              // rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn0 += 1;
              // rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn0 += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4;     
            } else {
              // Update the packet flow information
              rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn0 += 1;
              rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn0 += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4;
            }
          } else if (ect == ip::INET_ECN_CE){
            // The packet is already marked by other hops in the network
            rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
            rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - tcp_hdr->doff * 4;
          } else {
            // The packet is non-ECT flow, drop as a feedback
            if (rand() < rx.get()->drb_flow_state[drb_id].mark_classic) {
              // return;
            }
          }
        } else {
          // During the TCP handshake, we don't mark
          rx.get()->five_tuple_to_rtt[pkt_five_tuple].ingress_of_syn = ts.count();
        }
        
        // Insert the packet into the DRB queue
        drb_queue_update(*ipv4_hdr, drb_id, ts, pkt_five_tuple);
        update_drb_flow_state_tcp(*ipv4_hdr, *tcp_hdr, drb_id, ts);
      } else if (ipv4_hdr->protocol == 17) {
        udphdr* udp_hdr = (udphdr*)malloc(sizeof(udphdr));
        memcpy(udp_hdr, (*sdu_it).data()+sizeof(iphdr), sizeof(udphdr));
        ip::swap_udphdr(udp_hdr);
        pkt_five_tuple = ip::extract_five_tuple(*ipv4_hdr, *udp_hdr);

        // logger.log_debug("UDP header source {}, dst {}, len {}, check {}", 
        //                  udp_hdr->source,
        //                  udp_hdr->dest,
        //                  udp_hdr->len,
        //                  udp_hdr->check); 

        // Flow mapping: map 5 tuples of IP/TCP flow to DRB id.
        rx.get()->five_tuple_to_drb[pkt_five_tuple].drb_id = drb_id;

        uint8_t ect = ipv4_hdr->tos & ip::INET_ECN_MASK;
        if (ect == ip::INET_ECN_ECT_1){
          if (rand() < rx.get()->drb_flow_state[drb_id].mark_l4s) {
            rx.get()->perform_ip_mark((*sdu_it).data(), ipv4_hdr, drb_id, pkt_five_tuple);
            rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
            rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - sizeof(udphdr); 
          } else {
            rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn1 += 1;
            rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn1 += ipv4_hdr->tot_len - sizeof(iphdr) - sizeof(udphdr);  
          }
        }
        if (ect == ip::INET_ECN_ECT_0){
          if (rand() < rx.get()->drb_flow_state[drb_id].mark_classic) {
            rx.get()->perform_ip_mark((*sdu_it).data(), ipv4_hdr, drb_id, pkt_five_tuple);
            rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
            rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - sizeof(udphdr); 
          } else {
            rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ecn0 += 1;
            rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ecn0 += ipv4_hdr->tot_len - sizeof(iphdr) - sizeof(udphdr);
          } 
        }
        if (ect == ip::INET_ECN_CE){
          rx.get()->five_tuple_to_drb[pkt_five_tuple].pkts_with_ce += 1;
          rx.get()->five_tuple_to_drb[pkt_five_tuple].bytes_with_ce += ipv4_hdr->tot_len - sizeof(iphdr) - sizeof(udphdr);
        }
        // Insert the packet into the DRB queue
        drb_queue_update(*ipv4_hdr, drb_id, ts, pkt_five_tuple);
        update_drb_flow_state_udp(*ipv4_hdr, *udp_hdr, drb_id, ts);
      } else {
        // Insert the packet into the DRB queue
        drb_queue_update(*ipv4_hdr, drb_id, ts, pkt_five_tuple);
      }
      // logger.log_debug("IP Packet {}", tmp_string.str());
      sdu_it ++;
    }
    // logger.log_info("Finished TX PDU. pdu_len={}, qfi={}", sdu.length(), qos_flow_id);
    tx.get()->handle_sdu(std::move(sdu), qos_flow_id);
  }

  void create_rx() final
  {
    rx = std::make_unique<mark_entity_rx_impl>(ue_index, psi, rx_sdu_notifier);
  }

  void create_tx(mark_tx_pdu_notifier& tx_pdu_notifier) final
  {
    tx = std::make_unique<mark_entity_tx_impl>(ue_index, psi, tx_pdu_notifier);
  }

  void add_drb(drb_id_t drb_id, pdcp_rlc_mode rlc_mod) final
  {
    drb_rlc[drb_id] = rlc_mod;
    pdcp_sn_sizes[drb_id] = 0;
    pdcp_sn_maxs[drb_id] = 0;
    next_tx_id[drb_id] = 0;
    next_delivery_id[drb_id] = 0;
    rx->drb_flow_state[drb_id] = {};
  }

  void add_mapping(qos_flow_id_t qfi, drb_id_t drb_id) final
  {
    qfi_to_drb[qfi] = drb_id;
  }

  void set_pdcp_sn_size(drb_id_t drb_id, uint8_t sn_size) final {
    pdcp_sn_sizes[drb_id] = sn_size;
    pdcp_sn_maxs[drb_id] = 1 << sn_size;
  }

  // Handle the feedback from the NR-U interface, it's called by another executor,
  // so it won't affect the downlink or uplink traffic performance.
  void handle_feedback(mark_utils::delivery_status_feedback feedback, drb_id_t drb_id_) final 
  {
    logger.log_info("Received feedback for {}", drb_id_);
    
    bool change_mark_flag = false;
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    // TODO_HW: maybe remove some too old entries to save memory.

    if (feedback.highest_pdcp_sn_retransmitted != 0) {
      // TODO_HW: add wrap around determination
      // logger.log_debug("next_tx_id {}, current feedback {}", next_tx_id[drb_id_], feedback.highest_pdcp_sn_retransmitted);
      double total_size = 0;
      double total_time = 0;
      double dequeue_rate = 0;

      // Update the dequeue rate based on the feedback
      if (next_tx_id[drb_id_] == 0) {
        dequeue_rate = 0;
      } else {
        for (size_t i = next_tx_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
          if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_transmitted){
            total_size += drb_pdcp_sn_ts[drb_id_][i].size;
          }
        }
        total_time = (double)(timestamp - drb_pdcp_sn_ts[drb_id_][next_tx_id[drb_id_] - 1].transmitted_time).count();
        dequeue_rate = total_size / total_time;
        if (total_time < 1000) {
          dequeue_rate = drb_pdcp_sn_ts[drb_id_][next_tx_id[drb_id_]-1].cal_dequeue_rate;
        }
      }

      // Update the dequeue rate estimation errors
      for (size_t i = next_tx_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
        if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_retransmitted) {
          drb_pdcp_sn_ts[drb_id_][i].transmitted_time = timestamp;
          // if (dequeue_rate != 0) {
          drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate = dequeue_rate; //calculate_dequeue_rate(i, drb_id_);
          if (drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate > 0) {
            drb_pdcp_sn_ts[drb_id_][i].dequeue_rate_error = drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate - drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate;
            logger.log_debug("current_ts:{}, drb_id:{}, i:{}, ingress:{}, dequeue_rate_pred:{}, dequeue_rate_cal:{}, error_esti:{}, error:{}",
              timestamp.count(),
              drb_id_, i, drb_pdcp_sn_ts[drb_id_][i].ingress_time.count(), 
              drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate, drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate,
              drb_pdcp_sn_ts[drb_id_][i].est_dequeue_rate_error, drb_pdcp_sn_ts[drb_id_][i].dequeue_rate_error);
          }
          // } else {
          //   drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate = dequeue_rate; //calculate_dequeue_rate(i, drb_id_);
          // }
          // logger.log_debug("Current {} calculated dequeue rate {}", i, drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate);
          drb_pdcp_sn_ts[drb_id_][i].queue_delay = (timestamp - drb_pdcp_sn_ts[drb_id_][i].ingress_time).count();
          if (drb_pdcp_sn_ts[drb_id_][i].est_queue_delay > 0) {
            drb_pdcp_sn_ts[drb_id_][i].queue_delay_error = drb_pdcp_sn_ts[drb_id_][i].queue_delay - drb_pdcp_sn_ts[drb_id_][i].est_queue_delay;
          }
        } else {
          break;
        }
      }
      
      if (next_tx_id[drb_id_] < feedback.highest_pdcp_sn_retransmitted + 1) {
        change_mark_flag = true;
        next_tx_id[drb_id_] = feedback.highest_pdcp_sn_retransmitted + 1;
      }
    }

    if (feedback.highest_pdcp_sn_delivered_retransmitted != 0) {
      // logger.log_debug("next_delivery_id {}, current feedback {}", next_delivery_id[drb_id_], feedback.highest_pdcp_sn_delivered_retransmitted);
      // TODO_HW: add wrap around determination
      for (size_t i = next_delivery_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
        if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_delivered_retransmitted) {
          drb_pdcp_sn_ts[drb_id_][i].delivered_time = timestamp;
        } else {
          break;
        }
      }
      next_delivery_id[drb_id_] = feedback.highest_pdcp_sn_delivered_retransmitted + 1;
    }

    if (feedback.highest_pdcp_sn_transmitted != 0) {
      // logger.log_debug("next_tx_id {}, current feedback {}", next_tx_id[drb_id_], feedback.highest_pdcp_sn_transmitted);
      // TODO_HW: add wrap around determination
      double total_size = 0;
      double total_time = 0;
      double dequeue_rate = 0;

      if (next_tx_id[drb_id_] == 0) {
        dequeue_rate = 0;
      } else {
        // next_tx_id[drb_id_]-1 > 0
        for (size_t i = next_tx_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
          if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_transmitted){
            total_size += drb_pdcp_sn_ts[drb_id_][i].size;
          }
        }
        total_time = (double)(timestamp - drb_pdcp_sn_ts[drb_id_][next_tx_id[drb_id_] - 1].transmitted_time).count();
        dequeue_rate = total_size / total_time;
        if (total_time < 1000) {
          dequeue_rate = drb_pdcp_sn_ts[drb_id_][next_tx_id[drb_id_]-1].cal_dequeue_rate;
        }
      }

      for (size_t i = next_tx_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
        if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_transmitted){
          drb_pdcp_sn_ts[drb_id_][i].transmitted_time = timestamp;
          // if (dequeue_rate != 0) {
          drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate = dequeue_rate; //calculate_dequeue_rate(i, drb_id_);
          if (drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate > 0) {
            drb_pdcp_sn_ts[drb_id_][i].dequeue_rate_error = drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate - drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate;
            logger.log_debug("current_ts:{}, drb_id:{}, i:{}, ingress:{}, dequeue_rate_pred:{}, dequeue_rate_cal:{}, error_esti:{}, error:{}",
              timestamp.count(),
              drb_id_, i, drb_pdcp_sn_ts[drb_id_][i].ingress_time.count(), 
              drb_pdcp_sn_ts[drb_id_][i].pred_dequeue_rate, drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate,
              drb_pdcp_sn_ts[drb_id_][i].est_dequeue_rate_error, drb_pdcp_sn_ts[drb_id_][i].dequeue_rate_error);
          }
          // } else {
            // drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate = dequeue_rate; //calculate_dequeue_rate(i, drb_id_);
          // }
          // logger.log_debug("Current {} calculated dequeue rate {}", i, drb_pdcp_sn_ts[drb_id_][i].cal_dequeue_rate);
          drb_pdcp_sn_ts[drb_id_][i].queue_delay = (timestamp - drb_pdcp_sn_ts[drb_id_][i].ingress_time).count();
          if (drb_pdcp_sn_ts[drb_id_][i].est_queue_delay > 0) {
            drb_pdcp_sn_ts[drb_id_][i].queue_delay_error = drb_pdcp_sn_ts[drb_id_][i].queue_delay - drb_pdcp_sn_ts[drb_id_][i].est_queue_delay;
          }
        } else {
          break;
        }
      }
      if (next_tx_id[drb_id_] < feedback.highest_pdcp_sn_transmitted + 1) {
        change_mark_flag = true;
        next_tx_id[drb_id_] = feedback.highest_pdcp_sn_transmitted + 1;
      }
    }

    if (feedback.highest_pdcp_sn_delivered != 0) {
      // logger.log_debug("next_delivery_id {}, current feedback {}", next_tx_id[drb_id_] - 1, feedback.highest_pdcp_sn_transmitted);
      // TODO_HW: add wrap around determination
      for (size_t i = next_delivery_id[drb_id_]; i < drb_pdcp_sn_ts[drb_id_].size(); i++) {
        if (drb_pdcp_sn_ts[drb_id_][i].pdcp_sn <= feedback.highest_pdcp_sn_delivered) {
          drb_pdcp_sn_ts[drb_id_][i].delivered_time = timestamp;
        } else {
          break;
        }
      }
      next_delivery_id[drb_id_] = feedback.highest_pdcp_sn_delivered + 1;
    }

    if (change_mark_flag) {
      make_mark_decision(drb_id_);
    }
    logger.log_info("Finished feedback for {}", drb_id_);
  }

private:
  mark_session_logger   logger;
  uint32_t              ue_index;
  pdu_session_id_t      psi;
  mark_rx_sdu_notifier& rx_sdu_notifier;

  // std::unique_ptr<m1_cu_up_gateway_bearer> m1_gw_bearer;
  // std::unique_ptr<m1_bearer>               m1;

  std::unique_ptr<mark_entity_tx_impl> tx;
  std::unique_ptr<mark_entity_rx_impl> rx;

  /// @brief: QoS to DRB mapping
  std::unordered_map<qos_flow_id_t, drb_id_t> qfi_to_drb;

  /// @brief: Ingress queue state
  std::unordered_map<drb_id_t, std::vector<mark_utils::pdcp_sn_size_ts>> drb_pdcp_sn_ts;
  /// @brief: History queue state
  // std::unordered_map<drb_id_t, std::vector<mark_utils::pdcp_sn_size_ts>> drb_pdcp_sn_ts_egress;

  /// @brief A pointer to the current queue head, next packet to be delivered through RLC,
  /// absolute idx in drb_pdcp_sn_ts_ingress and drb_pdcp_sn_ts_egress.
  std::unordered_map<drb_id_t, size_t> next_delivery_id;
  /// @brief A pointer to the current queue head, next packet to be sent through RLC,
  /// absolute idx in drb_pdcp_sn_ts_ingress and drb_pdcp_sn_ts_egress.
  std::unordered_map<drb_id_t, size_t> next_tx_id;

  std::unordered_map<drb_id_t, uint32_t> next_pdcp_sn;

  /// @brief Initially, set the pdcp sequence bit size to be 18, if we find a wrap around at 4096,
  /// we reset this to 12.
  std::unordered_map<drb_id_t, uint8_t> pdcp_sn_sizes;
  std::unordered_map<drb_id_t, uint32_t> pdcp_sn_maxs;

  std::unordered_map<drb_id_t, pdcp_rlc_mode> drb_rlc;

  /// @brief window side used to calculate the packet dequeue rate
  size_t dequeue_rate_cal_wind;
  /// @brief window side used to predict the packet dequeue rate
  size_t dequeue_rate_pred_wind;

  double* dequeue_history;

  double* dequeue_xpred;
  double* dequeue_amse;

  double l4s_tq_thr;
  double classic_tq_thr;
  uint32_t n_max = 1500*20;

  // called upon receiving a downlink packet
  void drb_queue_update(iphdr ipv4_hdr, drb_id_t drb_id, std::chrono::microseconds now, ip::five_tuple f_tuple)
  {
    mark_utils::pdcp_sn_size_ts new_pdcp_pkt = {};
    new_pdcp_pkt.pdcp_sn = next_pdcp_sn[drb_id] % pdcp_sn_maxs[drb_id];
    next_pdcp_sn[drb_id] += 1;
    new_pdcp_pkt.size = ipv4_hdr.tot_len;
    new_pdcp_pkt.ingress_time = now;
    new_pdcp_pkt.five_tuple = f_tuple;
    drb_pdcp_sn_ts[drb_id].push_back(new_pdcp_pkt);
  }

  void update_drb_flow_state_tcp(iphdr ipv4_hdr, tcphdr hdr, drb_id_t drb_id, std::chrono::microseconds now) {
    if (ip::classify_flow(ipv4_hdr) == ip::L4S_FLOW) {
      // logger.log_debug("l4s drb_id {}, hdr.syn {}, {}", drb_id, (uint8_t)hdr.syn, ((uint8_t)hdr.syn) == 0);
      if (ipv4_hdr.protocol == 6 && ((uint8_t)hdr.syn) == 0) {
        rx.get()->drb_flow_state[drb_id].have_l4s = true;
        rx.get()->drb_flow_state[drb_id].l4s_last_see = now;
      }
    } else {
      // logger.log_debug("classic drb_id {}, hdr.syn {}, {}", drb_id, (uint8_t)hdr.syn, ((uint8_t)hdr.syn) == 0);
      if (ipv4_hdr.protocol == 6 && ((uint8_t)hdr.syn) == 0){
        rx.get()->drb_flow_state[drb_id].have_classic = true;
        rx.get()->drb_flow_state[drb_id].classic_last_see = now;
      }
        
    }

    // flow liveness check
    if ((now - rx.get()->drb_flow_state[drb_id].l4s_last_see).count() > 1000000) 
      rx.get()->drb_flow_state[drb_id].have_l4s = false;

    if ((now - rx.get()->drb_flow_state[drb_id].classic_last_see).count() > 1000000) 
      rx.get()->drb_flow_state[drb_id].have_classic = false;
  }

  void update_drb_flow_state_udp(iphdr ipv4_hdr, udphdr hdr, drb_id_t drb_id, std::chrono::microseconds now) {
    if (ip::classify_flow(ipv4_hdr) == ip::L4S_FLOW) {
      // logger.log_debug("l4s drb_id {}, udp", drb_id);
      if (ipv4_hdr.protocol == 17) {
        rx.get()->drb_flow_state[drb_id].have_l4s = true;
        rx.get()->drb_flow_state[drb_id].l4s_last_see = now;
      }
    } else {
      // logger.log_debug("classic drb_id {}, udp", drb_id);
      if (ipv4_hdr.protocol == 17) {
        rx.get()->drb_flow_state[drb_id].have_classic = true;
        rx.get()->drb_flow_state[drb_id].classic_last_see = now;
      }
    }

    // flow liveness check
    if ((now - rx.get()->drb_flow_state[drb_id].l4s_last_see).count() > 1000000) 
      rx.get()->drb_flow_state[drb_id].have_l4s = false;

    if ((now - rx.get()->drb_flow_state[drb_id].classic_last_see).count() > 1000000) 
      rx.get()->drb_flow_state[drb_id].have_classic = false;
  }

  // update the dequeue rate calculation up to i-th packet, in bytes per micro-second
  double calculate_dequeue_rate(size_t index, drb_id_t drb_id) {
    if (index == 0) {
      // for the first data, we can't calculate the dequeue rate
      return 0;
    }

    double total_sz = 0;

    if (index < dequeue_rate_cal_wind) {
      for (size_t i = 1; i <= index; i ++ ){
        total_sz += drb_pdcp_sn_ts[drb_id][i].size;
      }
      return total_sz / (double)(drb_pdcp_sn_ts[drb_id][index].transmitted_time - 
        drb_pdcp_sn_ts[drb_id][0].transmitted_time).count();
    } else {
      for (size_t i = index - dequeue_rate_cal_wind + 1; i <= index; i ++) {
        total_sz += drb_pdcp_sn_ts[drb_id][i].size;
      }
      return total_sz / (double)(drb_pdcp_sn_ts[drb_id][index].transmitted_time - 
        drb_pdcp_sn_ts[drb_id][index - dequeue_rate_cal_wind].transmitted_time).count();
    }
  }

  /// @brief Predict the sending for the queue tail packet's queuing delay.
  /// The time series window size is dequeue_rate_pred_wind, the latest observation is next_tx_id[drb]-1,
  /// and the unobserved data is from next_tx_id[drb] to drb_pdcp_sn_ts[drb].queue_tail().
  /// At least, we should use the 1 to dequeue_rate_pred_wind as the observation window.
  void predict_dequeue_rate(drb_id_t drb_id) 
  { 
    auto& vec = drb_pdcp_sn_ts[drb_id];
    if (vec.empty()) return;
    
    if (next_tx_id[drb_id] == 0) {
      // we don't expect to reach this 
      drb_pdcp_sn_ts[drb_id][next_tx_id[drb_id]].pred_dequeue_rate = 0;
      return;
    }

    if (next_tx_id[drb_id] - 1 < dequeue_rate_pred_wind) {
      // current window size is smaller than the dequeue rate predition window size
      // fallback to just an average over the past dequeue rate
      double pred_dq_rate = 0;
      double dq_std = 0;
      for (size_t i = 1; i < next_tx_id[drb_id]; i ++) {
        pred_dq_rate += drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate / (double)(next_tx_id[drb_id] - 1);
      }
      for (size_t i = 1; i < next_tx_id[drb_id]; i ++) {
        dq_std += (pred_dq_rate - drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate) * (pred_dq_rate - drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate) / (double)(next_tx_id[drb_id] - 1);
      }
      dq_std = sqrt(dq_std);

      drb_pdcp_sn_ts[drb_id].back().pred_dequeue_rate = pred_dq_rate;
      drb_pdcp_sn_ts[drb_id].back().est_dequeue_rate_error = dq_std;
      // logger.log_debug("Finished predicting the average dqueue rate {} bytes / ns.", 
        // pred_dq_rate);
    } else {
      double pred_dq_rate = 0;
      double dq_std = 0;
      for (size_t i = next_tx_id[drb_id] - dequeue_rate_pred_wind; i < next_tx_id[drb_id]; i ++) {
        pred_dq_rate += drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate / (double)dequeue_rate_pred_wind;
      }
      for (size_t i = next_tx_id[drb_id] - dequeue_rate_pred_wind; i < next_tx_id[drb_id]; i ++) {
        dq_std += (pred_dq_rate - drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate) * (pred_dq_rate - drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate) / (double)dequeue_rate_pred_wind;
      }
      dq_std = sqrt(dq_std);

      drb_pdcp_sn_ts[drb_id].back().pred_dequeue_rate = pred_dq_rate;
      drb_pdcp_sn_ts[drb_id].back().est_dequeue_rate_error = dq_std;
      // logger.log_debug("Finished predicting the average dqueue rate {} bytes / ns.", 
        // pred_dq_rate);

    //   // predict with the ARIMA first, then shift to the PID controller method
    //   logger.log_debug("Start predicting using the ARIMA method.");
    //   for (size_t i = next_tx_id[drb_id] - dequeue_rate_pred_wind; i < next_tx_id[drb_id]; i ++) {
    //     dequeue_history[i-next_tx_id[drb_id]+dequeue_rate_pred_wind] = drb_pdcp_sn_ts[drb_id][i].cal_dequeue_rate;
    //   }
    //   arima_exec(arima_obj, dequeue_history);
    //   logger.log_debug("Finished training the ARIMA method.");
    //   arima_summary(arima_obj);
    //   dequeue_xpred = (double*)calloc(drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id], sizeof(double));
    //   dequeue_amse = (double*)calloc(drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id], sizeof(double));
    //   arima_predict(arima_obj, dequeue_history, drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id], dequeue_xpred, dequeue_amse);
    //   logger.log_debug("Finished predicting the ARIMA method dqueue rate {} bytes / ns.", 
    //     dequeue_xpred[drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id]-1]);

    //   drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].pred_dequeue_rate = dequeue_xpred[drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id] - 1];
    //   drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].est_dequeue_rate_error = dequeue_amse[drb_pdcp_sn_ts[drb_id].size() - next_tx_id[drb_id] - 1];
    }
  }

  void predict_queuing_delay(drb_id_t drb_id) 
  {
    double standing_queue_sz = 0;
    for (size_t i = next_tx_id[drb_id]; i < drb_pdcp_sn_ts[drb_id].size(); i ++) {
      standing_queue_sz += drb_pdcp_sn_ts[drb_id][i].size;
    }
    drb_pdcp_sn_ts[drb_id].back().standing_queue_size = standing_queue_sz;
    drb_pdcp_sn_ts[drb_id].back().est_queue_delay = standing_queue_sz / drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].pred_dequeue_rate;
  }

  void make_mark_decision(drb_id_t drb_id) 
  {
    // only update the queue tail's dequeue rate and calculate the queuing delay.
    predict_dequeue_rate(drb_id);
    
    // predict the standing packets' average queuing delay, based on that we make the marking decision.
    predict_queuing_delay(drb_id);

    // change mark decision
    double required_dequeue_rate = drb_pdcp_sn_ts[drb_id].back().standing_queue_size / l4s_tq_thr;
    double predicted_dequeue_rate = drb_pdcp_sn_ts[drb_id].back().pred_dequeue_rate;
    double predicted_error = drb_pdcp_sn_ts[drb_id].back().est_dequeue_rate_error;
    double predicted_qdely = drb_pdcp_sn_ts[drb_id].back().est_queue_delay;

    logger.log_debug("required_dequeue_rate {}, predicted_dequeue_rate {}, predicted_error {}, est_dequeue_time {}, queue_size {}", 
      required_dequeue_rate, 
      predicted_dequeue_rate, 
      predicted_error,
      predicted_qdely,
      drb_pdcp_sn_ts[drb_id].back().standing_queue_size);

    rx.get()->nof_ue = nof_ue;
    // L4S flow only
    if (rx.get()->drb_flow_state[drb_id].have_l4s) {
    // && !rx.get()->drb_flow_state[drb_id].have_classic){
      if (required_dequeue_rate >  predicted_dequeue_rate + predicted_error) {
        rx.get()->drb_flow_state[drb_id].mark_l4s = RAND_MAX;
      } else if (required_dequeue_rate < predicted_dequeue_rate - predicted_error) {
        rx.get()->drb_flow_state[drb_id].mark_l4s = 0;
      } else {
        rx.get()->drb_flow_state[drb_id].mark_l4s = (uint32_t)(required_dequeue_rate - predicted_dequeue_rate + predicted_error) / 2 / predicted_error * RAND_MAX;
      }
    }

    // Classic flow only
    // double real_classic_tq_thr = classic_tq_thr * sqrt(nof_ue);
    // double required_dequeue_rate_classic = drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].standing_queue_size / real_classic_tq_thr;
    // double predicted_dequeue_rate_classic = drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].pred_dequeue_rate;
    // double predicted_error_classic = drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].est_dequeue_rate_error;
    if (rx.get()->drb_flow_state[drb_id].have_classic) {
      //&& !rx.get()->drb_flow_state[drb_id].have_l4s) {

      uint32_t classic_thres = n_max / nof_ue;
      if (drb_pdcp_sn_ts[drb_id].back().standing_queue_size > classic_thres) {
        // Final design
        // rx.get()->drb_flow_state[drb_id].mark_classic = ((drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].standing_queue_size - (double) classic_thres) / (double)classic_thres) / 100 *  // * RAND_MAX; 
        //      ((drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].standing_queue_size - (double)classic_thres) / (double)classic_thres)  / 100 * RAND_MAX;

        // Final design opt-2
        rx.get()->drb_flow_state[drb_id].mark_classic = (1460 * 8 * 1.75 / 2 / predicted_dequeue_rate / predicted_qdely) * 
          (1460 * 8 * 1.75 / 2 / predicted_dequeue_rate / predicted_qdely) * RAND_MAX;
              // ((drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].standing_queue_size - (double) classic_thres) / (double)classic_thres) / 100 *  // * RAND_MAX; 
              // ((drb_pdcp_sn_ts[drb_id][drb_pdcp_sn_ts[drb_id].size()-1].standing_queue_size - (double)classic_thres) / (double)classic_thres)  / 100 * RAND_MAX;
      } else {
        rx.get()->drb_flow_state[drb_id].mark_classic = 0;
      }

      // /* end_to_end_azure_0523_classic100ms_3 */
      // if (required_dequeue_rate_classic >  predicted_dequeue_rate_classic + predicted_error_classic) {
      //   rx.get()->drb_flow_state[drb_id].mark_classic = RAND_MAX;
      // } else if (required_dequeue_rate_classic < predicted_dequeue_rate_classic - predicted_error_classic) {
      //   rx.get()->drb_flow_state[drb_id].mark_classic = 0;
      // } else {
      //   rx.get()->drb_flow_state[drb_id].mark_classic = (uint32_t)(required_dequeue_rate_classic - predicted_dequeue_rate_classic + predicted_error_classic) / 10 / predicted_error_classic *
      //     (required_dequeue_rate_classic - predicted_dequeue_rate_classic + predicted_error_classic) / 10 / predicted_error_classic * RAND_MAX;
      // }
    }

    // Mixed flows for L4S and Classic
    if (rx.get()->drb_flow_state[drb_id].have_l4s && rx.get()->drb_flow_state[drb_id].have_classic) {
      /// TBD
    }
  }

public:
  
};

} // namespace srs_cu_up

} // namespace srsran
