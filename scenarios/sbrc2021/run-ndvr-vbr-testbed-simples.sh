#!/bin/bash

mkdir /tmp/minindn
sudo python examples/ndvr-paper.py --vbr --no-cli --ctime 60 --nPings 180 topologies/testbed.conf 2>&1 | tee /tmp/minindn/mndn.log
NOW=$(date +%Y%m%d%H%M%S)
echo "Finished at $NOW" | tee -a /tmp/minindn/mndn.log
mkdir -p /home/italo/results/ndvr-vbr-testbed-simples
sudo mv /tmp/minindn /home/italo/results/ndvr-vbr-testbed-simples/$NOW
sudo unlink /home/italo/results/ndvr-vbr-testbed-simples/last
sudo ln -s /home/italo/results/ndvr-vbr-testbed-simples/$NOW /home/italo/results/ndvr-vbr-testbed-simples/last
