#!/usr/bin/env python
#-*- coding:utf-8 -*-

from fluid.base import OFServer, OFConnection
from fluid.msg import of10

table = {}

class LearningSwitch(OFServer):
    def __init__(self):
        OFServer.__init__(self, "0.0.0.0", 6653, 1)

    def connection_callback(self, conn, event_type):
        try:
            if event_type == OFConnection.EVENT_STARTED:
                print "Connection id=%i started" % conn.get_id()
            elif event_type == OFConnection.EVENT_ESTABLISHED:
                print "Connection id=%i established" % conn.get_id()
            elif event_type == OFConnection.EVENT_FAILED_NEGOTIATION:
                print "Connection id=%i failed version negotiation" % conn.get_id()
            elif event_type == OFConnection.EVENT_CLOSED:
                print "Connection id=%i closed by the user" % conn.get_id()
            elif event_type == OFConnection.EVENT_DEAD:
                print "Connection id=%i closed due to inactivity" % conn.get_id()
        except Exception,e:
            print e
            
    def message_callback(self, conn, type_, data, length):
        try:
            if type_ == 10: # Packet in
                conn_id = conn.get_id()
                pi = of10.PacketIn()
                pi.unpack(data) 
                packet = of10.cdata(pi.data(), pi.data_len())
                dl_dst, dl_src = packet[:6], packet[6:12]                 
                # Flood broadcast
                if dl_dst == '\xff\xff\xff\xff\xff\xff':
                    self.flood(conn, pi)
                else:
                    if conn_id not in table:
                        table[conn_id] = {}
                    # Learn that dl_src is at in_port
                    table[conn_id][dl_src] = pi.in_port()
                    # If we don't know where the destination is, flood
                    if dl_dst not in table[conn_id]:
                        self.flood(conn, pi)
                    # Otherwise, install a new flow connecting src and dst
                    else:
                          port = table[conn_id][dl_dst]
                          m = of10.Match()
                          m.dl_src(of10.EthAddress(of10.btom(dl_src)))
                          m.dl_dst(of10.EthAddress(of10.btom(dl_dst)))
                          msg = of10.FlowMod()
                          msg.cookie(123)
                          msg.command(0)
                          msg.priority(5)
                          msg.idle_timeout(60)
                          msg.hard_timeout(300)
                          act = of10.OutputAction(port, 1024)
                          msg.add_action(act)
                          msg.match(m)
                          msg.buffer_id(pi.buffer_id())
                          buff = msg.pack()
                          conn.send(buff, msg.length())
        except Exception,e:
            print e;

    def flood(self, conn, pi):
        act = of10.OutputAction(0xfffb, 1024)
        msg =  of10.PacketOut(pi.xid(), pi.buffer_id(), pi.in_port())
        msg.add_action(act)
        msg.data(pi.data(), pi.data_len())        
        buff = msg.pack()
        conn.send(buff, msg.length())

c = LearningSwitch()
c.start(False)
raw_input("Press Enter to stop.")
c.stop()
