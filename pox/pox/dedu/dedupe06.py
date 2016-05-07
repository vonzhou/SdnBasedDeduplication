
"""
A l2 learning switch , Install flow 

Every FP pakcet(dst_port=XX) should go to controller

All use UDP packet for hijack the flow.

Recv Bloom Filter Bitset from server

bug:  cannot send packet_in directly to server 

vonzhou  2015.11.9
"""

from pox.core import core
import pox.openflow.libopenflow_01 as of
import pox.lib.packet as pkt
import time
from pox.lib.addresses import IPAddr, EthAddr
import fpcache

# import numpy as np
# from bitstring import BitArray
import bloomfilter as bloom
import deducore 

log = core.getLogger()

FP_PORT = 9877  # dedu app port
BLOOM_PORT = 10000  # for sync bloom filter and cache
BLOOM_SIZE = 50000  # TUNE THIS PARAMETER EFFECT, default = 50K

bf = bloom.BloomFilter(BLOOM_SIZE)

#  I want enum type
class QueryStatus:
  NEW_FILE = 0
  DUPLICATED_FILE = 1
  GO_TO_SERVER = 2
  INVALID = -1


class Injector (object):
  """
  A Injector object is created for each switch that connects.
  Doing learning switch 
  A Connection object for that switch is passed to the __init__ function.
  Detect our action packet to do dedupe
  """
  def __init__ (self, connection):
    self.switches = {}
    # Keep track of the connection to the switch so that we can
    # send it messages!
    self.connection = connection

    # This binds our PacketIn event listener
    connection.addListeners(self)

    # Use this table to keep track of which ethernet address is on
    # which switch port (keys are MACs, values are ports).
    self.mac_to_port = {}


  def resend_packet (self, packet_in, out_port):
    """
    Instructs the switch to resend a packet that it had sent to us.
    "packet_in" is the ofp_packet_in object the switch had sent to the
    controller due to a table-miss.
    """
    msg = of.ofp_packet_out()
    msg.data = packet_in
    action = of.ofp_action_output(port = out_port)
    msg.actions.append(action)
    self.connection.send(msg)

  def send_packetin_to_server(self, packet_in):
    """
    Send this pakcet in to switch nearest the SERVER(h10)
    TODO
    """
    msg = of.ofp_packet_out()
    msg.data = packet_in
    action = of.ofp_action_output(port = of.OFPP_ALL)   ## TODO 
    msg.actions.append(action)

    # for con in core.openflow.connections:
      #  print type(con.dpid),'----------------------------'

    con_s10 = core.openflow.getConnection(10)
    con_s10.send(msg)

  def duplication_detect(self, packet, packet_in):
    """
    Monitor and Parse the packet to get fp
    return  - new file 
            - hit our cache, duplicated
            - go to server, not sure, send this packet_in to server quickly
            - gerneral fp go to server  (?? If need bypass the network )
            -  stop, just for recv msg 

    """
    global bf  # Need Update 
    res = 3
    ip = packet.find('ipv4')
    if ip is None:
      return 
    srcip = ip.srcip
    dstip = ip.dstip
    # log.debug('before find udp , ip is type={}, srcip={}, dstip={}***'.format(ip.protocol, srcip, dstip))
    udp = ip.find('udp')
    if udp is None:
      return 
    #log.debug('srcport={},dstport={}'.format(tcp.srcport, tcp.dstport))

    dstport = udp.dstport
    fp = ''
    if dstport == FP_PORT: 
      udp_payload = udp.payload 
      # The payload include a '\n' , and its type is <type 'str'>
      #log.debug('tcp playload(len={}): {},type={}'.format(len(tcp_payload), str(tcp_payload), type(tcp_payload)))
      # parse fingerprint from the packet
      if len(udp_payload) >= 40:
        # log.debug('===fp + filename ={}'.format(udp_payload))
        fp = udp_payload[:40]
        # log.debug('{}::This file fp={}'.format(time.time(), fp))

        # dedu by the cache we maintain in controller
        
        res_bloom = bf.lookup(fp) # Bloom filter not find, then new file 
        # New file , tell client

        #############################################
        # 2016-2-23 TODO: reconstruct the code below :send different msg
        # 
        #########################################

        if res_bloom == False:
          # log.debug('---------------{} is a new file'.format(fp))
          ether_pkt = create_udp_pkt(dstip, srcip, udp.srcport, packet.src, "new_file")
          msg = of.ofp_packet_out()
          msg.data = ether_pkt.pack()
          # print 'port type:', type(packet_in.in_port)
          # print 'port :', packet_in.in_port
          msg.actions.append(of.ofp_action_output(port = packet_in.in_port))
          
          self.connection.send(msg)
          return QueryStatus.NEW_FILE

        exists = fpcache.lookup2(fp)
        # if exists then reply to the client, stop transfer quickly...
        if exists :
          ether_pkt = create_udp_pkt(dstip, srcip, udp.srcport, packet.src, "duplicated")
          msg = of.ofp_packet_out()
          msg.data = ether_pkt.pack()
          msg.actions.append(of.ofp_action_output(port = packet_in.in_port))
          
          self.connection.send(msg)
          # log.debug('{}::packet out from POX to host {}'.format(time.time(), srcip))
          return QueryStatus.DUPLICATED_FILE
          
        # else , go to server, costly
        # WE ISSUE THIS PACKET TO THE SERVER SIDE, QUICKLY   (TODO)
        ether_pkt = create_udp_pkt(dstip, srcip, udp.srcport, packet.src, "go to server")
        msg = of.ofp_packet_out()
        msg.data = ether_pkt.pack()
        msg.actions.append(of.ofp_action_output(port = packet_in.in_port))
          
        self.connection.send(msg)
        return QueryStatus.GO_TO_SERVER

      else: # udp payload len < 40
        print udp_payload, '===================='
        return QueryStatus.INVALID

    elif dstport == BLOOM_PORT:
      # update bloom filter from server
      deducore.sync_from_server(udp.payload, bf, fpcache)
      # bf = deducore.sync_bloomfilter_from_server(udp.payload)
    
    else:
      # TODO
      pass

    return res
        

  def send_msg_to_host(self):
    pass # TODO.....



  def act_like_switch (self, packet, packet_in):
    """
    Implement switch-like behavior.
    ether type:
    IP_TYPE    = 0x0800 = 2048
    ARP_TYPE   = 0x0806 = 2054
    IPV6_TYPE  = 0x86dd = 34525
    """
    # log.debug('[*] Packet type={},src={},dst={},in-port={},miss-len={}'.format(packet.type, packet.src, packet.dst, packet_in.in_port, packet_in._total_len))

    # Learn the port for the source MAC, mac,in_port pair
    self.mac_to_port[packet.src] = packet_in.in_port
    dst_port = self.mac_to_port.get(packet.dst)
    # dst_port = None  # test
    if dst_port is not None:
      #log.debug('Packet destinated to {} should go to port {}'.format(packet.dst, dst_port))

      # Send packet out the associated port
      self.resend_packet(packet_in, dst_port)
      #log.debug("Installing flow ({},{}) to ({},{})".format(packet.src, packet_in.in_port, packet.dst, dst_port))
      msg = of.ofp_flow_mod()
      
      # Set fields to match received packet
      msg.match = of.ofp_match.from_packet(packet)
      msg.idle_timeout = 0  # Not deleted
      msg.match.in_port = packet_in.in_port
      msg.priority = 0 # not OFP_DEFAULT_PRIORITY
      
      action = of.ofp_action_output(port = dst_port)
      msg.actions.append(action)
      self.connection.send(msg)
    else:
      # Flood the packet out other ports but the input port
      self.resend_packet(packet_in, of.OFPP_ALL)

  def packet_in_for_action(self, packet, packet_in):
    """
    The packet meet our CONTROLLER action get here.
    the fp packet need to be hijack,others just resend out
    """
    case = self.duplication_detect(packet, packet_in)
    
  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch
    """
    packet = event.parsed # This is the parsed packet data.
    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp #  ofp_packet_in message
    reason = packet_in.reason
    if reason == 1: # action
      self.packet_in_for_action(packet, packet_in)
    elif reason == 0: # no match 
      self.act_like_switch(packet, packet_in)


def launch ():
  """
  Starts the component
  """
  def start_switch (event):
    log.debug("Controlling %s" % (event.connection,))
    Injector(event.connection)

  
  def exit_switch(event):
    log.debug("[*] %s is down" % (event.connection))

  # When comes a switch connection, fired a ConnectionUp event.
  # THe Connection object is the channel between switch and controller 
  core.openflow.addListenerByName("ConnectionUp", start_switch)
  core.openflow.addListenerByName("ConnectionDown", exit_switch)

def create_udp_pkt(srcip, dstip, dstport, dstmac, data):
  """
  construct UDP packet to <dstmac, dstip, dstport>
  """
  payload = data
  udp_pkt = pkt.udp()
  udp_pkt.srcport = 10000
  udp_pkt.dstport = dstport
  udp_pkt.payload = payload

  ipv4_pkt = pkt.ipv4()
  ipv4_pkt.protocol = pkt.ipv4.UDP_PROTOCOL
  ipv4_pkt.srcip = srcip
  ipv4_pkt.dstip = dstip
  ipv4_pkt.payload = udp_pkt

  ether_pkt = pkt.ethernet()
  ether_pkt.dst = dstmac
  ether_pkt.src = EthAddr("00:11:22:33:44:55")
  ether_pkt.type = ether_pkt.IP_TYPE
  ether_pkt.payload = ipv4_pkt

  return ether_pkt