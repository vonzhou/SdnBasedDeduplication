#!/bin/bash

sudo killall -9 ovs-vswitchd

sudo killall -9 ovsdb-server 

sudo rmmod openvswitch

sudo rm /usr/local/etc/openvswitch/conf.db

