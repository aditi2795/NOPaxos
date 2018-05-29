# Arguments in order: protocol, #replicas, #threads per client, #client machines
# EX: python ./bench/runBench.py unreplicated 5 1 1
# To run with batching, use protocol "batch"
import sys, string
import subprocess
import os
from runBench import runTest
import matplotlib.pyplot as plt

clientMachines = 3
averageRuns = 1
protocols = ["unreplicated", "vr", "batch", "fastpaxos", "nopaxos"]
#protocols = ["unreplicated", "vr", "batch"]
#protocols = ["nopaxos"]
maxThreads = {"unreplicated": 12, "vr": 5, "batch": 10, "fastpaxos": 5,
        "nopaxos": 12}
for protocol in protocols:
    throughputList = []
    latencyList = []
    for threads in range(1, maxThreads[protocol]):
        avgThroughput = 0
        avgLatency = 0
        for i in range(averageRuns):
            throughput, latency = runTest(protocol, 3, threads, clientMachines)
            #change to 5 for real testing!
            if throughput == -1 and latency == -1:
                continue
            avgThroughput += throughput
            avgLatency += latency
        if avgThroughput == 0 and avgLatency == 0:
            continue
        avgThroughput /= float(averageRuns) 
        avgLatency /= float(averageRuns)
        throughputList.append(avgThroughput)
        latencyList.append(avgLatency)
    plt.plot(throughputList, latencyList, 'o', label=protocol)
plt.legend()
plt.xlabel("Throughput")
plt.ylabel("Latency")
plt.savefig('Figure5.png')

