cd ${0%/*}
cd ../bin/linux/Debug
for i in {1..100};
do
	./Robograde
	wait
done
