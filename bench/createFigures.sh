# Generate all figures
start=$(date)
echo "-----------------------------------"
echo "------ Figure 5, 3 replicas -------"
echo "-----------------------------------"
python createFig5-3.py
echo "-----------------------------------"
echo "------ Figure 5, 5 replicas -------"
echo "-----------------------------------"
python createFig5-5.py
echo "-----------------------------------"
echo "------------ Figure 8 -------------"
echo "-----------------------------------"
python createFig8.py
echo "-----------------------------------"
echo "------ Measure Bottleneck - -------"
echo "-----------------------------------"
python createBottleneckCheck.py
echo "START"
echo $start
echo "END"
date
