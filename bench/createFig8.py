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
maxThreads = {"unreplicated": 12, "vr": 5, "batch": 20, "fastpaxos": 4,
        "nopaxos": 12}
numReplicasList = [3, 5, 7, 9]
for protocol in protocols:
    throughputList = []
    for numReplicas in numReplicasList:
        avgThroughput = 0
        for i in range(averageRuns):
            throughput, latency = runTest(protocol, numReplicas,
                    maxThreads[protocol], clientMachines)
            if throughput == -1 and latency == -1:
                continue
            avgThroughput += throughput
        avgThroughput /= float(averageRuns) 
        throughputList.append(avgThroughput)
    plt.plot(numReplicasList, throughputList, label=protocol)
plt.legend()
plt.xlabel("Number of Replicas")
plt.ylabel("Throughput")
plt.savefig('Figure8.png')

