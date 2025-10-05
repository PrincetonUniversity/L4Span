#pragma once

#include <chrono>
#include "srsran/mark/ip_utils.h"


namespace mark_utils {

struct pdcp_sn_size_ts {
  uint32_t pdcp_sn = 0;
  size_t size = 0;
  std::chrono::microseconds ingress_time = {}; // When this packet enters the RAN stack
  std::chrono::microseconds transmitted_time = {}; // When this packet is transmitted by RLC layer
  std::chrono::microseconds delivered_time = {}; // When this packet is delivered to the UE, feedback by RLC 

  double standing_queue_size = 0; // current standing queue size when doing the prediction

  double cal_dequeue_rate = 0; // calculated dequeue rate when receiving the transmitted feedback
  double pred_dequeue_rate = 0; // predicted dequeue rate

  double queue_delay = 0; // acutal queuing delay by subtracting the ingress time and the tx time, in micro-second (us)
  double est_queue_delay = 0; // predicted queuing delay, in micro-second (us)
  
  double est_dequeue_rate_error = 0; // estimated dequeue rate error, when doing the prediction
  double dequeue_rate_error = 0; // actual dequeue rate error, when the packet is transmitted
 
  double queue_delay_error = 0; // queuing delay estimation error.

  ip::five_tuple five_tuple = {};
};

struct delivery_status_feedback {
  uint32_t highest_pdcp_sn_transmitted = 0;
  uint32_t highest_pdcp_sn_delivered = 0;
  uint32_t highest_pdcp_sn_retransmitted = 0;
  uint32_t highest_pdcp_sn_delivered_retransmitted = 0;
};

struct drb_flow_state {
  int mark_l4s = 0;
  int mark_classic = 0;
  bool have_l4s;
  bool have_classic;
  std::chrono::microseconds l4s_last_see;
  std::chrono::microseconds classic_last_see;
  // uint32_t have_l4s_udp = 0;
  // uint32_t have_classic_udp = 0;
};

} // namespace mark_utils


// Example of timestamp
// auto now = std::chrono::system_clock::now();
// auto duration = now.time_since_epoch();
// auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);

// std::cout << "Timestamp in microseconds: " << microseconds.count() << std::endl;