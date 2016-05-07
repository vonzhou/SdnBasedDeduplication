
"""
A l2 learning switch , but not install flow 

every pakcet should go to controller

get fp from the tcp packet 

vonzhou  2015.10.25
"""

from pox.core import core
import pox.openflow.libopenflow_01 as of
import pox.lib.packet as pkt

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

  def spy_tcp_packet(self, packet):
    ip = packet.find('ipv4')
    if ip is None:
      return
    #log.debug('protocol={},srcip={},dstip={}'.format(ip.protocol, ip.srcip, ip.dstip))

    # we only care about UDP packet
    tcp = ip.find('tcp')
    if tcp is None:
      return 

    #log.debug('srcport={},dstport={}'.format(tcp.srcport, tcp.dstport))

    dstport = tcp.dstport
    if dstport == 9878: # SERV_PORT + 1 
      tcp_payload = tcp.payload 
      # The payload include a '\n' , and its type is <type 'str'>
      #log.debug('tcp playload(len={}): {},type={}'.format(len(tcp_payload), str(tcp_payload), type(tcp_payload)))
      # parse fingerprint from the packet
      if len(tcp_payload) > 20:
        fp = tcp_payload[:20]
        log.debug('This file fp={}'.format(fp))

        # dedu by the cache we maintain in controller


  def act_like_switch (self, packet, packet_in):
    """
    Implement switch-like behavior.
    ether type:
    IP_TYPE    = 0x0800 = 2048
    ARP_TYPE   = 0x0806 = 2054
    IPV6_TYPE  = 0x86dd = 34525
    """


    log.debug('Packet type={},src={},dst={},in-port={}'.format(packet.type, packet.src, packet.dst, packet_in.in_port))

    self.spy_tcp_packet(packet)

    # Learn the port for the source MAC, mac,in_port pair
    self.mac_to_port[packet.src] = packet_in.in_port
    dst_port = self.mac_to_port.get(packet.dst)

    if dst_port is not None:
      log.debug('Packet destinated to {} should go to port {}'.format(packet.dst, dst_port))

      # Send packet out the associated port
      self.resend_packet(packet_in, dst_port)

      # Here I do not install flow entry , so every packet comes up 
    else:
      # Flood the packet out everything but the input port
      # This part looks familiar, right?
      log.debug('Donot know which port {} should go to, so I flood'.format(packet.dst))
      self.resend_packet(packet_in, of.OFPP_ALL)


  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch.
    """

    packet = event.parsed # This is the parsed packet data.
    #print dir(packet) #all members of packet object

    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp # The actual ofp_packet_in message.

    # Comment out the following line and uncomment the one after
    # when starting the exercise.
    #self.act_like_hub(packet, packet_in)
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
