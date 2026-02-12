#include "ring_buffer.h"

#define DEFAULT_MAX 262144


RingBuffer::RingBuffer(int nb){
    int val = nb; 
    if (nb == 0 || (nb & (nb - 1)) != 0){ 
        // If not a power of 2  => default value
        val = DEFAULT_MAX;
    }
    storage_.resize(val);       // nb must be a power of 2
    MASK = nb - 1;             // Mask
}

RingBuffer::RingBuffer(){
    RingBuffer(DEFAULT_MAX);
}


RingBuffer::~RingBuffer()=default;


void RingBuffer::add_element(mark_utils::pdcp_sn_size_ts element){
    // Adds an element at the tail (push)
    uint32_t sequence = element.pdcp_sn & MASK;
    storage_[sequence] = element;
    pdcp_ind_end_ = sequence;
}

void RingBuffer::push_back(mark_utils::pdcp_sn_size_ts element){
    add_element(element);
}


uint32_t RingBuffer::get_last_pdcp_ind(){
    // Returns PDCP seq num of element at the tail
    return pdcp_ind_end_;
}


mark_utils::pdcp_sn_size_ts& RingBuffer::get_element(uint32_t seq_num){
    // Access a specific packet
    uint32_t seq = seq_num & MASK;
    return storage_[seq];
}

mark_utils::pdcp_sn_size_ts& RingBuffer::operator[](uint32_t seq_num){
    // Access a specific packet
    return get_element(seq_num);
}

mark_utils::pdcp_sn_size_ts& RingBuffer::back(){
    // Returns packet at the tail
    return get_element( pdcp_ind_end_);
}

bool RingBuffer::empty(){
    return storage_.empty();
}