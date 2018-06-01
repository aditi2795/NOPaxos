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
protocols = ["nopaxos", "unreplicated", "vr", "batch", "fastpaxos"]
maxThreads = {"unreplicated": 12, "vr": 5, "batch": 20, "fastpaxos": 4,
        "nopaxos": 12}
legend = {"nopaxos": "NOPaxos", "unreplicated": "Unreplicated", "vr": "Paxos",
        "batch": "Batching", "fastpaxos": "Fast Paxos"}
numReplicasList = [3, 5, 7, 9]
for protocol in protocols:
    throughputList = []
    for numReplicas in numReplicasList:
        avgThroughput = 0
        totRuns = 0
        for i in range(averageRuns):
            throughput, latency,_ = runTest(protocol, numReplicas,
                    maxThreads[protocol], clientMachines)
            if throughput == -1 and latency == -1:
                continue
            totRuns += 1
            avgThroughput += throughput
        avgThroughput /= float(totRuns) 
        avgThroughput /= 1000   # change units to thousands
        throughputList.append(avgThroughput)
    plt.plot(numReplicasList, throughputList, label=legend[protocol], linestyle='-',
            marker='o')
plt.legend()
plt.xlabel("Number of Replicas")
plt.ylabel("Throughput (ops/sec)")
plt.gca().yaxis.set_major_formatter(FormatStrFormatter('%dK'))
plt.ylim([0,180])
plt.title("NOPAXOS Figure 8 replication")
plt.savefig('Figure8.png')

