#!/bin/sh

# See: http://git.openvswitch.org/cgi-bin/gitweb.cgi?p=openvswitch;a=blob_plain;f=INSTALL.SSL;hb=HEAD
CERT=/home/allanv/Desktop/certs/sc-cert.pem
PKEY=/home/allanv/Desktop/certs/sc-privkey.pem
CCRT=/usr/share/openvswitch/pki/controllerca/cacert.pem

SWITCH_NAME=switch
PORT1=b1.0
PORT2=b2.0

ovs-vsctl add-br $SWITCH_NAME
ovs-vsctl add-port $SWITCH_NAME $PORT1
ovs-vsctl add-port $SWITCH_NAME $PORT2

ovs-vsctl set-ssl $PKEY $CERT $CCRT
    
ovs-vsctl set-controller $SWITCH_NAME ssl:127.0.0.1:6653
ovs-vsctl set-fail-mode $SWITCH_NAME secure

echo "Switch with TLS running. Press Enter to stop."
read CONTINUE

ovs-vsctl del-br $SWITCH_NAME
