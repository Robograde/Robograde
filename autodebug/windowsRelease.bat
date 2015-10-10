cd ${0%/*}
cd ../bin/windows/Release
for i in {1..500};
do
	Robograde.exe
	wait
done