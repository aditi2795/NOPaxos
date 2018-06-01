# Evaluation of Network-Ordered Paxos on a Cloud Platform

This repository extends the [NOPaxos repository](https://github.com/UWSysLab/NOPaxos) by adding scripts to evaluate the performance of NOPaxos on a cloud platform. Specifically, it contains scripts to recreate figures 5 and 8 from the [NOPaxos paper](http://homes.cs.washington.edu/~lijl/papers/nopaxos-osdi16.pdf) on the gcloud platform.

Usage:
1. Email zbohn2@stanford.edu or evd2018@stanford.edu and request to be added to the [nopaxos-204404 gcloud project](https://console.cloud.google.com/compute/instances?project=nopaxos-204404); this project contains 5 client machines, 9 replica machines, and a sequencer, all fully configured. 
2. Clone this repository locally and cd into the NOPaxos directory.
3. To recreate all figures, execute: bench/createFigures.sh. This will create 4 .png files: "Figure5-3.png", "Figure 5-5.png", "Figure8.png", and "SeqBottleneck.png" in the current working directory. Be advised that this script takes >30 minutes to complete.


