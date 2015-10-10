cd ${0%/*}
cd ../bin/linux/Release
for i in {1..100};
do
	./Robograde
	wait
done
