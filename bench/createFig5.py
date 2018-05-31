# Arguments in order: protocol, #replicas, #threads per client, #client machines
# EX: python ./bench/runBench.py unreplicated 5 1 1
# To run with batching, use protocol "batch"
import sys, string
import subprocess
import os
from runBench import runTest
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter

clientMachines = 5
averageRuns = 3
legend = {"nopaxos": "NOPaxos", "unreplicated": "Unreplicated", "vr": "Paxos", "batch": "Batching", "fastpaxos": "Fast Paxos"}
protocols = ["nopaxos","unreplicated", "vr", "batch", "fastpaxos"]
#protocols = ["unreplicated", "vr", "batch"]
#protocols = ["nopaxos"]
maxThreads = {"unreplicated": 12, "vr": 5, "batch": 25, "fastpaxos": 4,
        "nopaxos": 20}
for protocol in protocols:
    throughputList = []
    latencyList = []
    for threads in range(1, maxThreads[protocol]):
        avgThroughput = 0
        avgLatency = 0
        for i in range(averageRuns):
            throughput, latency = runTest(protocol, 5, threads, clientMachines)
            #change to 5 for real testing!
            if throughput == -1 and latency == -1:
                continue
            avgThroughput += throughput
            avgLatency += latency
        if avgThroughput == 0 and avgLatency == 0:
            continue
        avgThroughput /= float(averageRuns)
        # change units to thousands
        avgThroughput /= 1000
        avgLatency /= float(averageRuns)
        throughputList.append(avgThroughput)
        latencyList.append(avgLatency)
    plt.plot(throughputList, latencyList, 'o', label=legend[protocol])
plt.legend()
plt.xlabel("Throughput (ops/sec)")
plt.xlim([0, 200])
plt.gca().xaxis.set_major_formatter(FormatStrFormatter('%dK'))
plt.ylabel("Latency (microsec)")
plt.title("NOPAXOS Figure 5 replication")
plt.savefig('Figure5.png')

