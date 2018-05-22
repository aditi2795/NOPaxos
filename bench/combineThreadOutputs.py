# usage:
# ./bench/client -c config -m unreplicated > output.txt
# python ./bench/combineThreadOutputs.py
import re

output_file = open("output.txt",'r')
file_text = output_file.read()

# Completed 100000 requests in 9.388013 seconds
p1 = re.compile("Completed ([0-9]+) requests in ([0-9\.]+) seconds");
match_iter1 = p1.finditer(file_text);
total_requests = 0;
max_seconds = -1;
for m in match_iter1:
    num_requests = float(m.group(1));
    total_requests += num_requests;
    num_seconds = float(m.group(2));
    if num_seconds > max_seconds:
        max_seconds = num_seconds;

if max_seconds != -1:
    print "Throughput (requests/sec): " + str(total_requests/max_seconds)
else:
    print "No throughput measurements"

# Average latency is 125968 ns (125 us)
p2 = re.compile("Average latency is [0-9]* ns \(([0-9]+) us\)");
match_iter2 = p2.finditer(file_text);
sum_avg_latency = 0;
num_avg_latency = 0;
for m in match_iter2:
    avg_latency = float(m.group(1));
    sum_avg_latency += avg_latency;
    num_avg_latency += 1;

if num_avg_latency > 0:
    print "Avg latency: " + str(sum_avg_latency/num_avg_latency);
else:
    print "No latency measurements"

