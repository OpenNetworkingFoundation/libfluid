#!/usr/bin/env python
#-*- coding:utf-8 -*-

from fluid.base import OFServer, OFServerSettings, OFConnection
from fluid.msg import of13

table = {}

class LearningSwitch(OFServer):
    def __init__(self):
        OFServer.__init__(self, "0.0.0.0", 6653, 1)
        config = OFServerSettings()
        config.supported_version(4)
	self.set_config(config)

    def message_callback(self, conn, type_, data, length):
        try:
            if type_ == 10: # Packet in
                conn_id = conn.get_id()
                pi = of13.PacketIn()
                pi.unpack(data) 
                in_port = pi.match().in_port().value() 
                packet = of13.cdata(pi.data(), pi.data_len())
                dl_dst, dl_src = packet[:6], packet[6:12]                
                # Flood broadcast
                if dl_dst == '\xff\xff\xff\xff\xff\xff':
                    self.flood(conn, pi, in_port)
                else:
                    if conn_id not in table:
                        table[conn_id] = {}
                    # Learn that dl_src is at in_port                        
                    table[conn_id][dl_src] = in_port
                    # If we don't know where the destination is, flood
                    if dl_dst not in table[conn_id]:
                        self.flood(conn, pi, in_port)
                    # Otherwise, install a new flow connecting src and dst
                    else:
                        port = table[conn_id][dl_dst]
                        fm = of13.FlowMod(pi.xid(), 123, 0xffffffffffffffff, 
                                          0,  0, 5, 60, 300, pi.buffer_id(), 
                                          0, 0, 0);
                        fsrc = of13.EthSrc(of13.EthAddress(of13.btom(dl_src))) 
                        fdst = of13.EthDst(of13.EthAddress(of13.btom(dl_dst))) 
                        fm.add_oxm_field(fsrc);
                        fm.add_oxm_field(fdst);
                        act = of13.OutputAction(port, 1024);
                        inst = of13.ApplyActions();
                        inst.add_action(act);
                        fm.add_instruction(inst);
                        buff = fm.pack();
                        conn.send(buff, fm.length());
            elif type_ == 6: # Features reply
                print "New datapath! Installing default flow."
                self.install_default_flow_mod(conn);

        except Exception,e:
            print "Exception",e;

    def flood(self, conn, pi, port):
        act = of13.OutputAction(0xfffffffb, 1024)
        msg =  of13.PacketOut(pi.xid(), pi.buffer_id(), port)
        msg.add_action(act)
        if (pi.buffer_id() == -1):
            msg.data(pi.data(), pi.data_len());       
        buff = msg.pack()
        conn.send(buff, msg.length())

    def install_default_flow_mod(self, conn):
        fm = of13.FlowMod(42, 0, 0xffffffffffffffff, 0, 0, 0, 0, 0,
                    0xffffffff, 0, 0, 0);
        act = of13.OutputAction(0xfffffffd, 0xffff);
        inst = of13.ApplyActions();
        inst.add_action(act);
        fm.add_instruction(inst);
        buff = fm.pack();
        conn.send(buff, fm.length());

c = LearningSwitch()
c.start(False)
raw_input("Press Enter to stop.")
c.stop()
