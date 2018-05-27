# Arguments in order: protocol, #replicas, #threads per client, #client machines
# EX: python ./bench/runBench.py unreplicated 5 1 1
import sys, string
import subprocess

def generateCmdStr(machine, remoteCmd):
    return ("gcloud compute --project \"%s\" ssh --zone \"%s\" \"%s\" --command \"%s\"") % (project, zone, machine, remoteCmd)

# Parse args
if len(sys.argv) < 5:
    print "Required arguments: protocol, number of replicas, number of threads per client, number of client machines"
protocol = sys.argv[1]
numReplicas = string.atoi(sys.argv[2])
numThreadsPerClient = string.atoi(sys.argv[3])
numClientMachines = string.atoi(sys.argv[4])

project = "nopaxos-204404"
zone = "us-west1-a"
configMap = {5: "config-5"} # TODO: add others
replicas = ["nopaxos-1", "nopaxos-2", "nopaxos-3", "nopaxos-04", "nopaxos-05", "nopaxos-06", "nopaxos-07", "nopaxos-08", "nopaxos-09"]
clients = ["client", "client-2", "client-3"]
config = configMap[numReplicas]
sequencer = "seqeuencer"
processes = []

# Start sequencer for nopaxos
if protocol == "nopaxos":
    sequencerCmd = ("sudo lsof -t -i udp:8000 | sudo xargs kill; cd /home/emmadauterman/NOPaxos; sudo ./sequencer/sequencer -C %s -c sequencer_config") % config
    process = subprocess.Popen(generateCmdStr(sequencer, sequencerCmd), shell=True) 
    proccesses.append(process)

# Start replicas
for i in range(0, numReplicas):
    replicaCmd = ("sudo lsof -t -i udp:8000 | sudo xargs kill; cd /home/emmadauterman/NOPaxos; ./bench/replica -c %s -i %d -m %s") % (config, i, protocol)
    process = subprocess.Popen(generateCmdStr(replicas[i], replicaCmd), shell=True)
    processes.append(process)

# Start clients
clientCmd = ("cd /home/emmadauterman/NOPaxos; rm output.txt; ./bench/client -t %d -c %s -m %s &> output.txt; python ./bench/combineThreadOutputs.py") % (numThreadsPerClient, config, protocol)
clientProcesses = []
totThroughput = 0.0
totLatency = 0.0
for i in range(0, numClientMachines):
    process = subprocess.Popen(generateCmdStr(clients[i], clientCmd), shell=True, stdout=subprocess.PIPE)
    clientProcesses.append(process)

for i in range(0, numClientMachines):
    output = clientProcesses[i].stdout.read()
    outputLines = output.splitlines()
    elems = outputLines[0].split(":")
    totThroughput += float(elems[1])
    elems = outputLines[1].split(":")
    totLatency += float(elems[1])

avgLatency = totLatency / numClientMachines

# Kill replicas and sequencer.
for process in processes :
    process.kill()

print ""
print "******************************************************************"
print ("Finished running %s with %d replicas with %d client machines each running %d threads") % (protocol, numReplicas, numClientMachines, numThreadsPerClient)
print ("Total throughput (requests/sec): %d") % (totThroughput)
print ("Average latency (us): %d") % (avgLatency)

