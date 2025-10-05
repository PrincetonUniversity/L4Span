#pragma once

#include "srsran/ran/lcid.h"
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <netinet/in.h>

#include <cstdint>
#include <cstring>
#include <iostream>

namespace ip {

#define DIVOPT 16777216

enum classification_results {
	CLASSIC_FLOW	= 0,	/* C queue */
	L4S_FLOW		  = 1,	/* L queue (scalable marking/classic drops) */
};

enum {
	INET_ECN_NOT_ECT = 0,
	INET_ECN_ECT_1 = 1,
	INET_ECN_ECT_0 = 2,
	INET_ECN_CE = 3,
	INET_ECN_MASK = 3,
};

struct five_tuple {
  uint32_t src_addr;
  uint32_t dst_addr;
  uint16_t src_port;
  uint16_t dst_port;
  uint8_t protocol;

  bool operator==(const five_tuple& other) const
  {
    return src_addr == other.src_addr and dst_addr == other.dst_addr and
           src_port == other.src_port and dst_port == other.dst_port and 
           protocol == other.protocol;
  }

  bool operator!=(const five_tuple& other) const
  {
    return !(other == *this);
  }
};

inline void swap_2_bytes(unsigned short* word) {
  unsigned char word_1, word_2;
  const unsigned char * src = reinterpret_cast<const unsigned char*>(word);
  memcpy(&word_1, src, 1);
  memcpy(&word_2, src+1, 1);
  *word = (((short)word_1) << 8) | (0x00ff & word_2);
}

inline void swap_4_bytes(unsigned int* word) {
  unsigned char word_1, word_2, word_3, word_4;
  const unsigned char * src = reinterpret_cast<const unsigned char*>(word);
  memcpy(&word_1, src, 1);
  memcpy(&word_2, src+1, 1);
  memcpy(&word_3, src+2, 1);
  memcpy(&word_4, src+3, 1);
  *word = (((uint16_t)word_1) << 24) | (0x00ff0000 & (((uint16_t)word_2) << 16)) | (0x0000ff00 & (((uint16_t)word_3) << 8)) | (0x000000ff & word_4);
}

/* Works directly on the memory to swap the iphdr data structure's 
   unsigned shore and unsigned int's byte order. Be careful when 
   passing the original packet pointer in, it may break the connection. */
inline void swap_iphdr(iphdr* ip_hdr) {
  /* there are 4 elements that have 2 bytes need swaping */
  /* tot_len, id, frag_off, check*/
  swap_2_bytes(&ip_hdr->tot_len);
  swap_2_bytes(&ip_hdr->id);
  swap_2_bytes(&ip_hdr->frag_off);
  swap_2_bytes(&ip_hdr->check);
  /* there are 2 elements that have 4 bytes need swaping */
  /* saddr, daddr */
  swap_4_bytes(&ip_hdr->saddr);
  swap_4_bytes(&ip_hdr->daddr);
}

/* Works directly on the memory to swap the tcphdr data structure's 
   unsigned shore and unsigned int's byte order. Be careful when 
   passing the original packet pointer in, it may break the connection. */
inline void swap_tcphdr(tcphdr* tcp_hdr) {
  /* there are 5 elements that have 2 bytes need swaping */
  /* source, dest, window, check, urg_ptr*/
  swap_2_bytes(&tcp_hdr->source);
  swap_2_bytes(&tcp_hdr->dest);
  swap_2_bytes(&tcp_hdr->window);
  swap_2_bytes(&tcp_hdr->check);
  swap_2_bytes(&tcp_hdr->urg_ptr);
  /* there are 2 elements that have 4 bytes need swaping */
  /* seq, ack_seq */
  swap_4_bytes(&tcp_hdr->seq);
  swap_4_bytes(&tcp_hdr->ack_seq);
}

/* Works directly on the memory to swap the udphdr data structure's 
   unsigned shore and unsigned int's byte order. Be careful when 
   passing the original packet pointer in, it may break the connection. */
inline void swap_udphdr(udphdr* udp_hdr) {
  /* there are 4 elements that have 2 bytes need swaping */
  /* source, dest, len, check*/
  swap_2_bytes(&udp_hdr->source);
  swap_2_bytes(&udp_hdr->dest);
  swap_2_bytes(&udp_hdr->len);
  swap_2_bytes(&udp_hdr->check);
}

inline int classify_flow(iphdr ipv4_hdr) {
  uint8_t ect = ipv4_hdr.tos & INET_ECN_MASK;
  if (ect & INET_ECN_ECT_1) {
    return L4S_FLOW;
  } else {
    return CLASSIC_FLOW;
  }
}

template <typename T> five_tuple extract_five_tuple(iphdr ipv4_hdr, T l4hdr) {
  five_tuple res = {};
  if ((ipv4_hdr.protocol == 6 && std::is_same_v<T, tcphdr>) || 
                (ipv4_hdr.protocol == 17 && std::is_same_v<T, udphdr>)) {
    res.src_port = l4hdr.source;
    res.dst_port = l4hdr.dest;
  } else {
    return res;
  }
  res.src_addr = ipv4_hdr.saddr;
  res.dst_addr = ipv4_hdr.daddr;
  res.protocol = ipv4_hdr.protocol;
  return res;
}

/* Extract the 5 tuple and reverse the src and dst addr, the src and dst port to 
   map the ACK back to the downlink PKT */
template <typename T> five_tuple extract_five_tuple_for_ack(iphdr ipv4_hdr, T l4hdr) {
  five_tuple res = {};
  if ((ipv4_hdr.protocol == 6 && std::is_same_v<T, tcphdr>) || 
                (ipv4_hdr.protocol == 17 && std::is_same_v<T, udphdr>)) {
    res.src_port = l4hdr.dest;
    res.dst_port = l4hdr.source;
  } else {
    return res;
  }
  res.src_addr = ipv4_hdr.daddr;
  res.dst_addr = ipv4_hdr.saddr;
  res.protocol = ipv4_hdr.protocol;
  return res;
}

struct drb_tcp_state {
  srsran::drb_id_t drb_id;
  size_t bytes_with_ecn1 = 1;
  size_t bytes_with_ecn0 = 1;
  size_t bytes_with_ce = 0;
  size_t pkts_with_ecn1 = 0;
  size_t pkts_with_ecn0 = 0;
  size_t pkts_with_ce = 5;

  size_t current_ce_counter_pkt = 5;

  // The actual ACK size is size = tcphdr->ack_seq - ack_raw.
  size_t ack_raw = -1;
};

struct rtt_estimates
{
  int64_t ingress_of_syn = 0;
  int64_t ingress_of_second = 0;
  int64_t estimated_rtt = 0;
};


/* set ip checksum of a given ip header*/
inline uint16_t compute_ip_checksum(struct iphdr* iphdrp){
  uint32_t sum = 0;
  // version, ihl, tos
  sum += ((uint16_t)iphdrp->version) << 12;
  sum += ((uint16_t)iphdrp->ihl) << 8;
  sum += ((uint16_t)iphdrp->tos);

  // total length
  sum += iphdrp->tot_len;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // id
  sum += iphdrp->id;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // flags, fragment offset
  sum += ((uint16_t)iphdrp->frag_off);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // ttl, protocol
  sum += ((uint16_t) iphdrp->ttl) << 8;
  sum += ((uint16_t) iphdrp->protocol);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // source address
  sum += (uint16_t)(iphdrp->saddr >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(iphdrp->saddr);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // dst address
  sum += (uint16_t)(iphdrp->daddr >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(iphdrp->daddr);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  return ~((uint16_t)sum);
}

/* set ip checksum of a given ip header*/
inline uint16_t compute_tcp_checksum(struct iphdr* iphdrp, struct tcphdr* tcp_hdr, uint8_t* data){
  size_t total_len = iphdrp->tot_len; // total IP packet length in bytes
 
  uint32_t sum = 0;
  // source address
  sum += (uint16_t)(iphdrp->saddr >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(iphdrp->saddr);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // dst address
  sum += (uint16_t)(iphdrp->daddr >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(iphdrp->daddr);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // ipv4 protocol 
  sum += ((uint16_t) iphdrp->protocol);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // tcp segment length
  sum += (uint16_t)(total_len - sizeof(iphdr));
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // tcp fixed header
  // source port
  sum += (uint16_t)tcp_hdr->source;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // destination port
  sum += (uint16_t)tcp_hdr->dest;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // sequence number
  sum += (uint16_t)(tcp_hdr->seq >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(tcp_hdr->seq);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // ack number
  sum += (uint16_t)(tcp_hdr->ack_seq >> 16);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }
  sum += (uint16_t)(tcp_hdr->ack_seq);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // data offset, reserved, cwr, ece, urg, ack, psh, rst, syn, fin
  sum += ((uint16_t)tcp_hdr->doff) << 12;
  sum += ((uint16_t)tcp_hdr->res1) << 8;
  sum += ((uint16_t)tcp_hdr->cwr) << 7;
  sum += ((uint16_t)tcp_hdr->ece) << 6;
  sum += ((uint16_t)tcp_hdr->urg) << 5;
  sum += ((uint16_t)tcp_hdr->ack) << 4;
  sum += ((uint16_t)tcp_hdr->psh) << 3;
  sum += ((uint16_t)tcp_hdr->rst) << 2;
  sum += ((uint16_t)tcp_hdr->syn) << 1;
  sum += ((uint16_t)tcp_hdr->fin);
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // window
  sum += tcp_hdr->window;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // checksum skipped
  // urgent pointer
  sum += tcp_hdr->urg_ptr;
  if (sum > 0xffff) {
    sum = (uint16_t)sum + 1;
  }

  // option the same as the data
  auto* data_ptr = data;
  // now at the starting point of the option
  data_ptr = data_ptr + sizeof(iphdr) + sizeof(tcphdr); 
  size_t offset = sizeof(iphdr) + sizeof(tcphdr);

  while(offset < iphdrp->tot_len) {
    sum += ((uint16_t)*data_ptr) << 8;
    data_ptr += 1;
    offset += 1;
    if (sum > 0xffff) {
      sum = (uint16_t)sum + 1;
    }
    sum += ((uint16_t)*data_ptr);
    data_ptr += 1;
    offset += 1;
    if (sum > 0xffff) {
      sum = (uint16_t)sum + 1;
    }
  }

  return ~((uint16_t)sum);
}

} // namespace ip



template <>
struct std::hash<ip::five_tuple>
{
  std::size_t operator()(const ip::five_tuple& k) const
  {
    using std::size_t;
    using std::hash;
    using std::string;

    // Compute individual hash values for first,
    // second and third and combine them using XOR
    // and bit shifting:

    return ((((((hash<uint32_t>()(k.src_addr)
             ^ (hash<uint32_t>()(k.dst_addr) << 1)) >> 1)
             ^ (hash<uint16_t>()(k.src_port) << 1)) >> 1)
             ^ (hash<uint16_t>()(k.dst_port) << 1)) >> 1)
             ^ (hash<uint8_t>()(k.protocol));
  }
};

