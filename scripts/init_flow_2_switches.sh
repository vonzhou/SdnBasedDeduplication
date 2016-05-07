#!/bin/bash

#add flow, let fp packet go to SDN controller

#sudo ovs-ofctl add-flow s1 tcp,tp_dst=9878,idle_timeout=0,hard_timeout=0,priority=0,actions=controller

##########################################################################
# Op flows of s1(clent side):
# 1) drop IPv6, DHCP
# 2) fp datagram go to controller
#########################################################################
sudo ovs-ofctl del-flows s1 

sudo ovs-ofctl add-flow s1 ipv6,actions=drop
sudo ovs-ofctl add-flow s1 udp,tp_dst=68,actions=drop
sudo ovs-ofctl add-flow s1 udp,tp_dst=67,actions=drop

sudo ovs-ofctl add-flow s1 udp,tp_dst=9877,idle_timeout=0,hard_timeout=0,priority=0,actions=controller


##########################################################################
# Op flows of s2(server side):
# 1) drop IPv6, DHCP
# 2) sync bloomfilter and fp datagram go to controller
#########################################################################
sudo ovs-ofctl del-flows s2 

sudo ovs-ofctl add-flow s2 ipv6,actions=drop
sudo ovs-ofctl add-flow s2 udp,tp_dst=68,actions=drop
sudo ovs-ofctl add-flow s2 udp,tp_dst=67,actions=drop

sudo ovs-ofctl add-flow s2 udp,tp_dst=10000,idle_timeout=0,hard_timeout=0,priority=0,actions=controller
