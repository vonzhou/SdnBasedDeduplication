
from pox.lib import *
from  pox.core import core  # to import core module to register and hook the module
#from  pox.lib import packet as pkt #to import modules from packet
import pox.openflow.libopenflow_01 as of
from pox.lib.revent import *
from pox.lib.util import dpid_to_str
import pox.lib.packet as pkt
import os

log = core.getLogger()

class l2_learning(EventMixin):
	def __init__(self,transparent):
		self.listenTo(core.openflow)
		self.transparent = transparent

	def _handle_ConnectionUp(self,event):#everytime a connection is established, a new switch instance is created
		L2firewall(event.connection,self.transparent)

class L2firewall(object):
	def __init__(self,connection,transparent):
	# as soon as the module is hooked initialise..
		self.macaddrtable = {}
		self.transparent = transparent
		self.connection = connection
		connection.addListeners(self)

	def _handle_PacketIn (self,event):
	 # event handler for incoming packets.check pox/openflow/__init__.py for infor on PacketIn(Event) class. ofp represents the real openflow packet which triggered the event and is an event attribute.
		self.processPacket(event)

	def checkRules(self,packet):

		filter_ip_list = {'10.0.0.1','10.0.0.2'};
			
		if packet.find('ethernet'):
			srcmac = str(packet.src)
			dstmac = str(packet.dst)
		if packet.find('ipv4') != None  or packet.find('ipv6') != None:
			if packet.find('ipv4') != None:
				ippkt = packet.find('ipv4')
			elif packet.find('ipv6') != None:
				ippkt = packet.find('ipv6') 
			srcip = str(ippkt.srcip)
			dstip = str(ippkt.dstip)

		# do the filter
		if packet.type == packet.IP_TYPE and ((srcip == '10.0.0.1' and dstip == '10.0.0.2') or (srcip == '10.0.0.2' and dstip == '10.0.0.1')):
			return 'DROP';
			
	
		return 'SWITCH'

	def floodPacket(self,event):
		message = of.ofp_packet_out()
		message.actions.append(of.ofp_action_output(port=of.OFPP_FLOOD))
		message.data = event.data
		message.in_port = event.port
		return message
	

	def dropPacket(self,event):
		message = of.ofp_packet_out()
		message.data = event.data
		message.in_port = event.port
		return message

	def updateMap(self,srcportin,srcmacaddr): # to update the mac-addr, port table
		self.macaddrtable[srcmacaddr] = srcportin


	def sendFlowMod(self,msg,event):
		event.connection.send(msg)

	def processPacket(self,event):# frame flow entries to forward the packets based on entries made in macaddrtable
		parsedpkt = event.parsed
		log.debug("%i --> %s" ,event.dpid,parsedpkt)
		inport = event.port
		#log.debug("%s-->%s",inport,parsedpkt)
		dstmacaddr = parsedpkt.dst
		srcmacaddr = parsedpkt.src
		self.updateMap(inport,srcmacaddr)
		msg = of.ofp_flow_mod()#default setting
		msg.match = of.ofp_match.from_packet(parsedpkt,inport)
		msg.data = event.ofp

		if not self.transparent:
			if parsedpkt.type == parsedpkt.LLDP_TYPE or dstmacaddr.isBridgeFiltered():
				msg = self.dropPacket(event)
				return	
					
		if dstmacaddr.is_multicast: # if mulicast packet, then flood
			msg = self.floodPacket(event)

		elif self.checkRules(parsedpkt)=='DROP':
		#since no action is being set, the 'msg' will have a drop action

			pass

		elif dstmacaddr not in self.macaddrtable:#if destmac not in macaddrtable,flood
			msg = self.floodPacket(event)

		elif dstmacaddr in self.macaddrtable:# if dstmac in macaddrtable
			dstport = self.macaddrtable[dstmacaddr] #choose port
			if dstport == event.port: #if same as inport , drop the packet
				msg = self.dropPacket(event)
				print "dropping"
			elif dstport != event.port or self.checkRules(parsedpkt) == 'SWITCH': #else, insert a flow table entry
				msg.actions.append(of.ofp_action_output(port = dstport))
		#		log.debug ("%s"%msg)
		#
		self.sendFlowMod(msg,event)
		

def launch():
	print "in launch.."
	core.registerNew(l2_learning,False) #registering the component to the core














