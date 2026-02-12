#include <cstdint>
#include <vector>
#include <chrono>
#include <iostream>
#include "srsran/mark/mark_utils.h"

    
class RingBuffer
{
    public:
		RingBuffer(int nb_elems);    // nb_elems is either 262144 (18 bits) or 4096 (12 bits)
        RingBuffer();               // Default constructor; for 18 bits 
		~RingBuffer();
		void add_element(mark_utils::pdcp_sn_size_ts element);
        void push_back(mark_utils::pdcp_sn_size_ts element);
        uint32_t get_last_pdcp_ind();
        mark_utils::pdcp_sn_size_ts& get_element(uint32_t seq_num);
        mark_utils::pdcp_sn_size_ts& operator[](uint32_t seq_num);
        mark_utils::pdcp_sn_size_ts& back();
        bool empty();
        
    private:
        uint32_t MASK;              // To compute the %
	    uint32_t pdcp_ind_end_=0;   // Queue tail
        
	    std::vector<mark_utils::pdcp_sn_size_ts> storage_;	
};
    