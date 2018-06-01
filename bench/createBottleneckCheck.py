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
averageRuns = 5 
maxThreads = 12
numReplicasList = [3, 5, 7, 9]
throughputList = []
seqThroughputList = []
for numReplicas in numReplicasList:
    avgThroughput = 0
    avgSeqThroughput = 0
    totRuns = 0
    for i in range(averageRuns):
        throughput, latency, seqThroughput = runTest("nopaxos", numReplicas,
                maxThreads, clientMachines)
        if throughput == -1 and latency == -1:
            continue
        totRuns += 1
        avgThroughput += throughput
        avgSeqThroughput += seqThroughput
    avgThroughput /= float(totRuns)
    avgThroughput /= 1000
    avgSeqThroughput /= float(totRuns)
    avgSeqThroughput /= 1000
    throughputList.append(avgThroughput)
    seqThroughputList.append(avgSeqThroughput)
plt.plot(numReplicasList, throughputList, linestyle='-', marker='o', label="NOPaxos")
plt.plot(numReplicasList, seqThroughputList, linestyle='-', marker='o', label="Sequencer")
plt.legend()
plt.xlabel("Number of Replicas")
plt.ylabel("Throughput (ops/sec)")
plt.gca().yaxis.set_major_formatter(FormatStrFormatter('%dK'))
plt.title("NOPAXOS Sequencer as a Bottleneck")
plt.savefig('SeqBottleneck.png')

