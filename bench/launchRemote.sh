OUTPUT1=$(gcloud compute --project "nopaxos-204404" ssh --zone "us-west1-a" "client-1" --command './bench/client -t $THREADS -c config-5 -m $PROTOCOL > output.txt; python ./bench/combineThreadOutputs.py')
OUTPUT2=$(gcloud compute --project "nopaxos-204404" ssh --zone "us-west1-a" "client-2" --command './bench/client -t $THREADS -c config-5 -m $PROTOCOL > output.txt; python ./bench/combineThreadOutputs.py')
OUTPUT3=$(gcloud compute --project "nopaxos-204404" ssh --zone "us-west1-a" "client-3" --command './bench/client -t $THREADS -c config-5 -m $PROTOCOL > output.txt; python ./bench/combineThreadOutputs.py')


