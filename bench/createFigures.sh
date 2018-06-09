# Generate all figures
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
echo ""
echo "-----------------------------------"
echo "TESTS COMPLETED"
echo "Figure5-3.png, Figure5-5.png, Figure8.png, and SeqBottleneck.png generated in current working directory."
