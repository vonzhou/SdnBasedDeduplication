
"""
A l2 learning switch , Install flow 

every FP pakcet(dst_port=XX) should go to controller

get fp from the tcp packet 

send udp to host

vonzhou  2015.10.28 to 11.3
"""

from pox.core import core
import pox.openflow.libopenflow_01 as of
import pox.lib.packet as pkt
import time
from pox.lib.addresses import IPAddr, EthAddr

log = core.getLogger()



class Tutorial (object):
  """
  A Tutorial object is created for each switch that connects.
  A Connection object for that switch is passed to the __init__ function.
  """
  def __init__ (self, connection):
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

    # Add an action to send to the specified port
    action = of.ofp_action_output(port = out_port)
    msg.actions.append(action)

    # Send openflow message to a switch
    self.connection.send(msg)

  def spy_fp_packet(self, packet, packet_in):
    """
    Monitor and Parse the packet to get fp
    """
    res = 2
    ip = packet.find('ipv4')
    if ip is None:
      return 
    srcip = ip.srcip
    dstip = ip.dstip

    tcp = ip.find('tcp')
    if tcp is None:
      log.debug('===have no tcp payload')
      return 
    #log.debug('srcport={},dstport={}'.format(tcp.srcport, tcp.dstport))

    dstport = tcp.dstport
    if dstport == 9878: # SERV_PORT + 1 
      tcp_payload = tcp.payload 
      # The payload include a '\n' , and its type is <type 'str'>
      #log.debug('tcp playload(len={}): {},type={}'.format(len(tcp_payload), str(tcp_payload), type(tcp_payload)))
      # parse fingerprint from the packet
      if len(tcp_payload) > 40:
        fp = tcp_payload[:40]
        log.debug('{}::This file fp={}'.format(time.time(), fp))

        # dedu by the cache we maintain in controller
        
        # isnew = False # Bloom filter
        # if isnew:
        #   ether_pkt = create_udp_pkt(dstip, srcip, 9999, packet.src, "new_file")
        #   msg = of.ofp_packet_out()
        #   msg.data = ether_pkt.pack()
        #   msg.actions.append(of.ofp_action_output(port = packet_in.in_port))
          
        #   self.connection.send(msg)
        #   log.debug('{}::packet out from POX to host {}'.format(time.time(), srcip))
        #   return 0

        exists = True # lookup the BF and cache  , fake
        # if exists then reply to the client, transfer complete quickly...
        if exists :
          ether_pkt = create_udp_pkt(dstip, srcip, 9999, packet.src, "duplicated")
          msg = of.ofp_packet_out()
          msg.data = ether_pkt.pack()
          msg.actions.append(of.ofp_action_output(port = packet_in.in_port))
          
          self.connection.send(msg)
          log.debug('{}::packet out from POX to host {}'.format(time.time(), srcip))
          res = 1
          
        # #else , go to server

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
    #log.debug('Packet type={},src={},dst={},in-port={},miss-len={}'.format(packet.type, packet.src, packet.dst, packet_in.in_port, packet_in._total_len))

    #self.spy_tcp_packet(packet)

    # Learn the port for the source MAC, mac,in_port pair
    self.mac_to_port[packet.src] = packet_in.in_port
    dst_port = self.mac_to_port.get(packet.dst)

    if dst_port is not None:
      log.debug('Packet destinated to {} should go to port {}'.format(packet.dst, dst_port))

      # Send packet out the associated port
      self.resend_packet(packet_in, dst_port)

      # Here I do install flow entry for general packet

      log.debug("Installing flow ({},{}) to ({},{})".format(packet.src, packet_in.in_port, packet.dst, dst_port))
      # Maybe the log statement should have source/destination/port? Yes

      msg = of.ofp_flow_mod()
      
      # Set fields to match received packet
      msg.match = of.ofp_match.from_packet(packet)
      
      #< Set other fields of flow_mod (timeouts? buffer_id?) >
      msg.idle_timeout = 60
      msg.match.in_port = packet_in.in_port
      msg.priority = 0 # not OFP_DEFAULT_PRIORITY
      
      #< Add an output action, and send -- similar to resend_packet() >
      action = of.ofp_action_output(port = dst_port)
      msg.actions.append(action)
      self.connection.send(msg)
    else:
      # Flood the packet out everything but the input port
      # This part looks familiar, right?
      #log.debug('Donot know which port {} should go to, so I flood'.format(packet.dst))
      self.resend_packet(packet_in, of.OFPP_ALL)

  def packet_in_for_action(self, packet, packet_in):
    """
    The packet meet our CONTROLLER action get here.
    the fp packet need to be hijack,others just resend out
    """
    self.mac_to_port[packet.src] = packet_in.in_port
    dst_port = self.mac_to_port.get(packet.dst)

    case = self.spy_fp_packet(packet, packet_in)
    log.debug('case --------------{}'.format(case))
    if case == 1:
      return 

    if dst_port is not None:
        #log.debug('------Packet destinated to {} should go to port {}'.format(packet.dst, dst_port))
        self.resend_packet(packet_in, dst_port)
        # Here I do NOT install flow entry for general packet
    else:
        #log.debug('------Donot know which port {} should go to, so I flood'.format(packet.dst))
        self.resend_packet(packet_in, of.OFPP_ALL)
    
    # if case == 0:
    #   # new , tell client to transfer
    #   return
    # elif case == 1:
    #   # duplicated, tell client to stop
    #   return 
    # elif case == 2:
    #   # maybe duplicated or other packets, go to server
      

      


  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch.
    """

    packet = event.parsed # This is the parsed packet data.
    #print dir(packet) #all members of packet object

    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp #  ofp_packet_in message.
    reason = packet_in.reason
    log.debug('{}::get packet_in, reason= {}, len='.format(time.time(), reason, packet_in._total_len))
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
    Tutorial(event.connection)

  # When comes a switch connection, fired a ConnectionUp event.
  # THe Connection object is the channel between switch and controller 
  core.openflow.addListenerByName("ConnectionUp", start_switch)

def create_udp_pkt(srcip, dstip, dstport, dstmac, data):
  """
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