# Arguments in order: protocol, #replicas, #threads per client, #client machines
# EX: python ./bench/runBench.py unreplicated 5 1 1
# To run with batching, use protocol "batch"
import sys, string
import subprocess
import os
from runBench import runTest
import matplotlib.pyplot as plt

clientMachines = 5
averageRuns = 1 
maxThreads = 12
numReplicasList = [3, 5, 7, 9]
throughputList = []
seqThroughputList = []
for numReplicas in numReplicasList:
    avgThroughput = 0
    avgSeqThroughput = 0
    for i in range(averageRuns):
        throughput, latency, seqThroughput = runTest("nopaxos", numReplicas,
                maxThreads, clientMachines)
        if throughput == -1 and latency == -1:
            continue
        avgThroughput += throughput
        avgSeqThroughput += seqThroughput
    avgThroughput /= float(averageRuns)
    avgSeqThroughput /= float(averageRuns)
    throughputList.append(avgThroughput)
    seqThroughputList.append(avgSeqThroughput)
plt.plot(numReplicasList, throughputList, label="nopaxos")
plt.plot(numReplicasList, seqThroughputList, label="sequencer")
plt.legend()
plt.xlabel("Number of Replicas")
plt.ylabel("Throughput")
plt.savefig('SeqBottleneck.png')

