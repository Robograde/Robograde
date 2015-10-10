cd ${0%/*}
cd ../bin/windows/Debug
for i in {1..1000};
do
	Robograde.exe
	wait
done