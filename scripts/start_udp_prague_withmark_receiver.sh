ip netns exec ue1 ./udp_prague_receiver -a 192.168.1.5 -p 12300 -c >> receiver_withmark_0.txt & 
ip netns exec ue2 ./udp_prague_receiver -a 192.168.1.5 -p 12301 -c >> receiver_withmark_1.txt & 
ip netns exec ue3 ./udp_prague_receiver -a 192.168.1.5 -p 12302 -c >> receiver_withmark_2.txt & 
ip netns exec ue4 ./udp_prague_receiver -a 192.168.1.5 -p 12303 -c >> receiver_withmark_3.txt & 
ip netns exec ue5 ./udp_prague_receiver -a 192.168.1.5 -p 12304 -c >> receiver_withmark_4.txt & 
ip netns exec ue6 ./udp_prague_receiver -a 192.168.1.5 -p 12305 -c >> receiver_withmark_5.txt & 
ip netns exec ue7 ./udp_prague_receiver -a 192.168.1.5 -p 12306 -c >> receiver_withmark_6.txt & 
ip netns exec ue8 ./udp_prague_receiver -a 192.168.1.5 -p 12307 -c >> receiver_withmark_7.txt & 
