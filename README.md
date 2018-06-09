# Evaluation of Network-Ordered Paxos on a Cloud Platform

This repository extends the [NOPaxos repository](https://github.com/UWSysLab/NOPaxos) by adding scripts to evaluate the performance of NOPaxos on a cloud platform. Specifically, it contains scripts to recreate figures 5 and 8 from the [NOPaxos paper](http://homes.cs.washington.edu/~lijl/papers/nopaxos-osdi16.pdf) on the gcloud platform.

Usage:
1. Email zbohn2@stanford.edu or evd2018@stanford.edu and request to be added to the [nopaxos-204404 gcloud project](https://console.cloud.google.com/compute/instances?project=nopaxos-204404); this project contains 5 client machines, 9 replica machines, and a sequencer, all fully configured. 
2. Clone this repository locally and cd into the NOPaxos directory.
3. To recreate all figures, execute: bench/createFigures.sh. This will create 4 .png files: "Figure5-3.png", "Figure 5-5.png", "Figure8.png", and "SeqBottleneck.png" in the current working directory. Be advised that this script takes ~70 minutes to complete.

To set up your own test environment, you will need 5 client machines, 1 sequencer, and 9 replicas. Make sure IP forwarding is enabled and the machines are all in the same zone. Run the following commands on each machine:
~~~~
sudo apt-get install git protobuf-compiler pkg-config libunwind-dev libssl-dev libprotobuf-dev libevent-dev libgtest-dev make g++ lsof
git clone https://github.com/edauterman/NOPaxos.git
cd NOPaxos
make PARANOID=0
~~~~ 
You will need to modify the configuration files (config-3, config-5, config-7, config-9, sequencer_config) to use the correct IP and MAC addresses and change the bench/runBench.py to ssh into the correct machines.


