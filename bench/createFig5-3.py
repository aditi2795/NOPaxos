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
averageRuns = 10
legend = {"nopaxos": "NOPaxos", "unreplicated": "Unreplicated", "vr": "Paxos", "batch": "Batching", "fastpaxos": "Fast Paxos"}
protocols = ["nopaxos","unreplicated", "vr", "batch", "fastpaxos"]
#maxThreads = {"unreplicated": 25, "vr": 5, "batch": 25, "fastpaxos": 4, "nopaxos": 20}
#stepSize = {"unreplicated": 3, "vr": 1, "batch": 3, "fastpaxos": 1, "nopaxos": 3}
threadRanges = {"unreplicated": range(1, 6, 1) + range(6, 25, 3), "vr": range(1,
    5), "batch": range(1, 25, 3), "fastpaxos": range(1, 4), "nopaxos": range(1, 20, 3)}
for protocol in protocols:
    throughputList = []
    latencyList = []
    for threads in threadRanges[protocol]:
        avgThroughput = 0
        avgLatency = 0
        totRuns = 0
        for i in range(averageRuns):
            throughput, latency,_ = runTest(protocol, 3, threads, clientMachines)
            if throughput == -1 and latency == -1:
                continue
            totRuns += 1
            avgThroughput += throughput
            avgLatency += latency
        if avgThroughput == 0 and avgLatency == 0:
            continue
        avgThroughput /= float(totRuns)
        # change units to thousands
        avgThroughput /= 1000
        avgLatency /= float(totRuns)
        throughputList.append(avgThroughput)
        latencyList.append(avgLatency)
    plt.plot(throughputList, latencyList, label=legend[protocol], linestyle='-',
            marker='o')
plt.legend()
plt.xlabel("Throughput (ops/sec)")
plt.xlim([0, 200])
plt.gca().xaxis.set_major_formatter(FormatStrFormatter('%dK'))
plt.ylabel("Latency (microsec)")
plt.title("NOPAXOS Figure 5 replication (3 replicas)")
plt.savefig('Figure5-3.png')

