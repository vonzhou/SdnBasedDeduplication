#sudo mn --topo=linear,10 --mac --switch=ovsk --controller=remote,ip=127.0.0.1


# Set link bandwidth 
#sudo mn --topo=linear,10 --link tc,bw=100,delay=10ms --mac --switch=ovsk --controller=remote,ip=127.0.0.1

sudo mn --topo=linear,10 --link tc,bw=1000,delay=1ms --mac --switch=ovsk --controller=remote,ip=127.0.0.1


