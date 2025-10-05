ip netns exec ue1 ./scream_bw_test_rx 192.168.1.5 12300 -ackdiff 1 >> receiver_withoutmark_0.txt & 
ip netns exec ue2 ./scream_bw_test_rx 192.168.1.5 12301 -ackdiff 1 >> receiver_withoutmark_1.txt & 
ip netns exec ue3 ./scream_bw_test_rx 192.168.1.5 12302 -ackdiff 1 >> receiver_withoutmark_2.txt & 
ip netns exec ue4 ./scream_bw_test_rx 192.168.1.5 12303 -ackdiff 1 >> receiver_withoutmark_3.txt & 
ip netns exec ue5 ./scream_bw_test_rx 192.168.1.5 12304 -ackdiff 1 >> receiver_withoutmark_4.txt & 
ip netns exec ue6 ./scream_bw_test_rx 192.168.1.5 12305 -ackdiff 1 >> receiver_withoutmark_5.txt & 
ip netns exec ue7 ./scream_bw_test_rx 192.168.1.5 12306 -ackdiff 1 >> receiver_withoutmark_6.txt & 
ip netns exec ue8 ./scream_bw_test_rx 192.168.1.5 12307 -ackdiff 1 >> receiver_withoutmark_7.txt & 