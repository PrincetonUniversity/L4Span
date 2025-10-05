#pragma once 

#include "mark_session_logger.h"
#include "srsran/mark/mark.h"
#include <netinet/in.h>


#include <sstream>
#include <iomanip>

namespace srsran{

namespace srs_cu_up{

class mark_entity_rx_impl : public mark_rx_pdu_handler
{
public:
  uint32_t nof_ue;

  mark_entity_rx_impl(uint32_t              ue_index,
                      pdu_session_id_t      psi,
                      mark_rx_sdu_notifier& sdu_notifier_) : 
    logger("MARK", {ue_index, psi, "UL"}), 
    sdu_notifier(sdu_notifier_)
    {
      five_tuple_to_drb = {};
      drb_flow_state = {};
      nof_ue = 1;
    }

  void handle_pdu(byte_buffer pdu, qos_flow_id_t qfi_) final
  {
    // pass through with qfi
    logger.log_info("RX SDU. {} sdu_len={}", qfi_, pdu.length());

    auto pdu_it = pdu.segments().begin();
    // std::stringstream tmp_string;
    // tmp_string << std::hex << std::setfill('0');
    while (pdu_it != pdu.segments().end()) 
    {
      // for (unsigned int buffer_i = 0; buffer_i < (*pdu_it).size(); buffer_i++) {
      //   tmp_string << std::hex << std::setw(2) << static_cast<int>((*pdu_it).data()[buffer_i]);
      // }

      ip::five_tuple pkt_five_tuple;
      iphdr* ipv4_hdr = (iphdr*)malloc(sizeof(iphdr));
      memcpy(ipv4_hdr, (*pdu_it).data(), sizeof(iphdr));
      drb_id_t drb_id;
      tcphdr* tcp_hdr = (tcphdr*)malloc(sizeof(tcphdr));
      udphdr* udp_hdr = (udphdr*)malloc(sizeof(udphdr));
      ip::swap_iphdr(ipv4_hdr);
      if (ipv4_hdr->protocol == 6) {
        memcpy(tcp_hdr, (*pdu_it).data()+sizeof(iphdr), sizeof(tcphdr));
        ip::swap_tcphdr(tcp_hdr);        
        pkt_five_tuple = ip::extract_five_tuple_for_ack(*ipv4_hdr, *tcp_hdr);
        drb_id = five_tuple_to_drb[pkt_five_tuple].drb_id;
        if (tcp_hdr->ack_seq > 0 && tcp_hdr->ack_seq < five_tuple_to_drb[pkt_five_tuple].ack_raw) {
          // The lowest ACK for ack raw + 1;
          five_tuple_to_drb[pkt_five_tuple].ack_raw = tcp_hdr->ack_seq - 1;
          // logger.log_debug("Lowest ack_raw {}", tcp_hdr->ack_seq - 1);
        }
        // logger.log_debug("Compute TCP checksum {}, actual checksum {}", 
        //   ip::compute_tcp_checksum(ipv4_hdr, tcp_hdr, (*pdu_it).data()),
        //   (uint16_t)tcp_hdr->check);

        /* Perform TCP option MARK */
        if (ip::classify_flow(*ipv4_hdr) == ip::L4S_FLOW && (uint8_t)tcp_hdr->ack) {
          // For uplink TCP option marking, the decision is made by the downlink 
          // logger.log_debug("Marking tcp ack header...");
          perform_tcp_mark((*pdu_it).data(), ipv4_hdr, pkt_five_tuple, tcp_hdr);
          // logger.log_debug("Finished marking tcp ack header...");
          
        } else if (ip::classify_flow(*ipv4_hdr) == ip::CLASSIC_FLOW && (uint8_t)tcp_hdr->ack){
          // For uplink TCP option marking, the decision is made by the downlink 
          perform_tcp_mark((*pdu_it).data(), ipv4_hdr, pkt_five_tuple, tcp_hdr);
          // perform_ip_mark((*pdu_it).data(), ipv4_hdr, drb_id, pkt_five_tuple);

        } else if (ip::classify_flow(*ipv4_hdr) == ip::L4S_FLOW && 
            drb_flow_state[drb_id].have_classic) {
          // logger.log_debug("L4S flow in hybrid DRB, TBD");
        } else if (ip::classify_flow(*ipv4_hdr) == ip::CLASSIC_FLOW && 
            drb_flow_state[drb_id].have_classic) {
          // logger.log_debug("Classic flow in hybrid DRB, TBD");
        }
        
      } else if (ipv4_hdr->protocol == 17) {
        memcpy(udp_hdr, (*pdu_it).data()+sizeof(iphdr), sizeof(udphdr));
        ip::swap_udphdr(udp_hdr);
        pkt_five_tuple = ip::extract_five_tuple_for_ack(*ipv4_hdr, *udp_hdr);
        drb_id = five_tuple_to_drb[pkt_five_tuple].drb_id;
      }
      
      pdu_it ++;
    }

    // logger.log_debug("IP packet bytes {}", tmp_string.str());
    // logger.log_info("Finished RX SDU. {} sdu_len={}", qfi_, pdu.length());
    sdu_notifier.on_new_sdu(std::move(pdu), qfi_);
  }

private:
  mark_session_trx_logger logger;
  mark_rx_sdu_notifier&   sdu_notifier;

public:
  void perform_ip_mark(uint8_t* pdu, iphdr* ipv4_hdr, drb_id_t drb_id, ip::five_tuple five_tuple) {    
    uint8_t ect = ipv4_hdr->tos & ip::INET_ECN_MASK;
    if (ect == ip::INET_ECN_CE) {
      // logger.log_debug("Packet already marked, abort...");
    } else if (ect == ip::INET_ECN_ECT_0) {
      // logger.log_debug("MARK ECT-0 (Classic) flow");    

      // IP-ECN set to 0x11 and update ipv4 header checksum
      ipv4_hdr->tos |= ip::INET_ECN_CE;
      ipv4_hdr->check = ip::compute_ip_checksum(ipv4_hdr);
      // logger.log_debug("IP packet marked.");
    } else if (ect == ip::INET_ECN_ECT_1) {
      // logger.log_debug("MARK ECT-1 (L4S) flow");
      
      // IP-ECN set to 0x11 and update ipv4 header checksum
      ipv4_hdr->tos |= ip::INET_ECN_CE;
      ipv4_hdr->check = ip::compute_ip_checksum(ipv4_hdr);
      // logger.log_debug("IP packet marked.");
    } else {
      ipv4_hdr->tos |= ip::INET_ECN_CE;
      ipv4_hdr->check = ip::compute_ip_checksum(ipv4_hdr);
    }
    ip::swap_iphdr(ipv4_hdr);
    memcpy(pdu, ipv4_hdr, sizeof(iphdr));
  }

  void perform_tcp_mark(uint8_t* pdu, iphdr* ipv4_hdr, ip::five_tuple five_tuple, tcphdr* tcp_hdr) {
    // update the TCP option field for the ACCECN requirement
    // if (tcp_hdr->doff < 8) {
    //   logger.log_debug("Not using TCP option, abort...");
    //   return;
    // } else {
    // tcp option field for r.ceb, r.ec0, r.ec1
    logger.log_debug("Copying AccECN if applicable...");
    uint8_t* opt_start = pdu + sizeof(iphdr) + sizeof(tcphdr);
    size_t offset = 0;
    size_t total_offset = tcp_hdr->doff * 4 - sizeof(tcphdr);

    size_t ce_pkt;
    size_t ce_bytes;
    size_t ecn0_bytes;
    size_t ecn1_bytes;
    size_t total_pkt = 0;
    size_t pkt_size = 1336;

    uint8_t ect = ipv4_hdr->tos & ip::INET_ECN_MASK;
    logger.log_debug("tcp ack {}", 
                    tcp_hdr->ack_seq);
    logger.log_debug("pkt ecn {}, ecn0 pkt {}, ecn1 pkt {}, ce pkt {}", 
                    ect,
                    five_tuple_to_drb[five_tuple].pkts_with_ecn0,
                    five_tuple_to_drb[five_tuple].pkts_with_ecn1,
                    five_tuple_to_drb[five_tuple].pkts_with_ce);
    if (ect == ip::INET_ECN_ECT_0 && five_tuple_to_drb[five_tuple].pkts_with_ecn0 > 0 && five_tuple_to_drb[five_tuple].pkts_with_ce > 5) {        
      total_pkt = (size_t)((tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw - 1) / pkt_size);
      double portion = (double)five_tuple_to_drb[five_tuple].bytes_with_ce / ((double)five_tuple_to_drb[five_tuple].bytes_with_ecn0 + (double)five_tuple_to_drb[five_tuple].bytes_with_ce);
      ce_pkt = (size_t)(total_pkt * portion) + 5;
      // if (ce_pkt > five_tuple_to_drb[five_tuple].current_ce_counter_pkt) {
      //   // Feedback the packet one by one
      //   five_tuple_to_drb[five_tuple].current_ce_counter_pkt ++;
      //   ce_pkt = five_tuple_to_drb[five_tuple].current_ce_counter_pkt;
      // }
      ce_bytes = (size_t)(ce_pkt * pkt_size - 5 * pkt_size) % DIVOPT;
      ecn0_bytes = (size_t)((tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw) - ce_bytes) % DIVOPT;
      ecn1_bytes = 1;
    } else if (ect == ip::INET_ECN_ECT_1 && five_tuple_to_drb[five_tuple].pkts_with_ecn1 > 0 && five_tuple_to_drb[five_tuple].pkts_with_ce > 5) {
      total_pkt = (size_t)((tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw - 1) / pkt_size);
      double portion = (double)five_tuple_to_drb[five_tuple].bytes_with_ce / ((double)five_tuple_to_drb[five_tuple].bytes_with_ecn1 + (double)five_tuple_to_drb[five_tuple].bytes_with_ce);
      portion /= 10;
      
      ce_pkt =  (size_t)(total_pkt * portion) + 5;
      // if (ce_pkt > five_tuple_to_drb[five_tuple].current_ce_counter_pkt) {
      //   // Feedback the packet one by one
      //   five_tuple_to_drb[five_tuple].current_ce_counter_pkt ++;
      //   ce_pkt = five_tuple_to_drb[five_tuple].current_ce_counter_pkt;
      // }
      ce_bytes = (size_t)(ce_pkt * pkt_size - 5 * pkt_size) % DIVOPT;
      ecn1_bytes = (size_t)((tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw) - ce_bytes) % DIVOPT;
      ecn0_bytes = 1;
      logger.log_debug("current counter {}, ce_pkt {}, ecn1 size {}, ce size {}", 
                    five_tuple_to_drb[five_tuple].current_ce_counter_pkt,
                    ce_pkt,
                    ecn1_bytes,
                    ce_bytes);
    } else {
      logger.log_debug("Don't change!");
      return;
    }

    logger.log_debug("total_pkt {}, ce pkt {}, ecn0 bytes {}, ecn1 bytes {}, ce bytes {}, ack bytes {}", 
                    total_pkt,
                    ce_pkt,
                    ecn0_bytes,
                    ecn1_bytes,
                    ce_bytes,
                    tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw);

    // double mark_portion;
    // size_t pkt_size =  five_tuple_to_drb[five_tuple].bytes_with_ecn0 / five_tuple_to_drb[five_tuple].pkts_with_ecn0;
    // size_t ce_bytes = 0;
    // size_t ack_bytes = tcp_hdr->ack_seq - five_tuple_to_drb[five_tuple].ack_raw;
    // if (ect == ip::INET_ECN_ECT_0) {
    //   mark_portion = (double)five_tuple_to_drb[five_tuple].bytes_with_ce / (double)five_tuple_to_drb[five_tuple].bytes_with_ecn0;
    // } else if (ect == ip::INET_ECN_ECT_1) {
    //   mark_portion = (double)five_tuple_to_drb[five_tuple].bytes_with_ce / (double)five_tuple_to_drb[five_tuple].bytes_with_ecn1;
    // }

    // least significant bits for r.cep
    tcp_hdr->res1 = ((ce_pkt & ( 1 << 2 )) >> 2);
    tcp_hdr->cwr = ((ce_pkt & ( 1 << 1 )) >> 1);
    tcp_hdr->ece = ((ce_pkt & ( 1 << 0 )) >> 0);

    // logger.log_debug("res1 {}, cwr {}, ece {}, ce_pkt {}", 
    //                  (uint8_t)tcp_hdr->res1,
    //                  (uint8_t)tcp_hdr->cwr,
    //                  (uint8_t)tcp_hdr->ece,
    //                  ce_pkt);
    while (offset < total_offset) {
      // logger.log_debug("offset {}, opt_start {}, total_offset {}", offset, *opt_start, total_offset);
      if (*opt_start == 1) {
        // TCP Option No-Operation, move one byte (8 bit)
        opt_start ++;
        offset ++;
      } else if (*opt_start == (uint8_t)174) {
        logger.log_debug("Found type 174...");
        // AccECN field we are looking for, directly overwrite it with our own tracked state.
        auto len = opt_start + 1;
        offset += (size_t)len;
        opt_start = opt_start + 2;
        // ECN 1 bytes
        *opt_start = (uint8_t)((ecn1_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ecn1_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ecn1_bytes & 0x0000ff));
        opt_start ++;

        // CE bytes
        *opt_start = (uint8_t)((ce_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ce_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ce_bytes & 0x0000ff));
        opt_start ++;

        // ECN 0 bytes
        *opt_start = (uint8_t)((ecn0_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ecn0_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ecn0_bytes & 0x0000ff));
        opt_start ++;

        break;
      } else if (*opt_start == (uint8_t)172) {
        logger.log_debug("Found type 172...");
        // AccECN field we are looking for, directly overwrite it with our own tracked state.
        auto len = opt_start + 1;
        offset += (size_t)len;
        opt_start = opt_start + 2;
        // ECN 0 bytes
        *opt_start = (uint8_t)((ecn0_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ecn0_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ecn0_bytes & 0x0000ff));
        opt_start ++;

        // CE bytes
        *opt_start = (uint8_t)((ce_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ce_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ce_bytes & 0x0000ff));
        opt_start ++;

        // ECN 1 bytes
        *opt_start = (uint8_t)((ecn1_bytes & 0xff0000) >> 16);
        opt_start ++;
        *opt_start = (uint8_t)((ecn1_bytes & 0x00ff00) >> 8);
        opt_start ++;
        *opt_start = (uint8_t)((ecn1_bytes & 0x0000ff));
        opt_start ++;

        break;
      } else {
        // other option field, we pass by
        auto len = *(opt_start + 1);
        offset += (size_t)len;
        opt_start += (size_t)len;
      }
    }
    // }
    // update the new TCP checksum
    auto sum = ip::compute_tcp_checksum(ipv4_hdr, tcp_hdr, pdu);
    tcp_hdr->check = sum;
    ip::swap_tcphdr(tcp_hdr);
    memcpy(pdu+sizeof(iphdr), tcp_hdr, sizeof(tcphdr));
  }

  void perform_udp_mark(uint8_t* pdu, iphdr* ipv4_hdr, drb_id_t drb_id, ip::five_tuple five_tuple, udphdr* udp_hdr) {
    logger.log_debug("MARK UDP's data gram, TBD!");
  }

};


} // namespace srs_cu_up

} // namespace srsran