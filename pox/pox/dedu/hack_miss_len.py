"""
vonzhou  2015.12.9
"""

from pox.core import core
import pox.openflow.libopenflow_01 as of
import pox.lib.packet as pkt
import time
from pox.lib.addresses import IPAddr, EthAddr
import fpcache

import numpy as np
# from bitstring import BitArray
import bloomfilter as bloom

log = core.getLogger()

FP_PORT = 9877
BLOOM_PORT = 10000
BLOOM_SIZE = 50000

bf = bloom.BloomFilter(BLOOM_SIZE)
cache = set()  # fp cache TODO

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
    """
    msg = of.ofp_packet_out()
    msg.data = packet_in
    action = of.ofp_action_output(port = of.OFPP_ALL)   ## TODO 
    msg.actions.append(action)

    # for con in core.openflow.connections:
      #  print type(con.dpid),'----------------------------'

    con_s10 = core.openflow.getConnection(10)
    con_s10.send(msg)

  def l7_detect(self, packet, packet_in):
    ip = packet.find('ipv4')
    if ip is None:
      return 
    srcip = ip.srcip
    dstip = ip.dstip
    # log.debug('before find udp , ip is type={}, srcip={}, dstip={}***'.format(ip.protocol, srcip, dstip))
    udp = ip.find('udp')
    if udp is None:
      return 
    udp_payload = udp.payload 
    #log.debug('srcport={},dstport={}'.format(tcp.srcport, tcp.dstport))

    log.debug('[*] {}'.format(type(udp_payload)))
      
    

  def act_like_switch (self, packet, packet_in):
    """
    Implement switch-like behavior.
    ether type:
    IP_TYPE    = 0x0800 = 2048
    ARP_TYPE   = 0x0806 = 2054
    IPV6_TYPE  = 0x86dd = 34525
    """
    log.debug('[*] Packet type={},in-port={},packet-in total_len={}, packet_in data len={}'.
      format(packet.type, packet_in.in_port, packet_in._total_len, len(packet_in.data)))

    # self.l7_detect(packet, packet_in)

    
    self.resend_packet(packet_in, of.OFPP_ALL)

  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch.
    """
    packet = event.parsed # This is the parsed packet data.
    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp #  ofp_packet_in message.
    reason = packet_in.reason
    # log.debug('{}::get packet_in, reason= {}, len='.format(time.time(), reason, packet_in._total_len))
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

  # When comes a switch connection, fired a ConnectionUp event.
  # THe Connection object is the channel between switch and controller 
  core.openflow.addListenerByName("ConnectionUp", start_switch)

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