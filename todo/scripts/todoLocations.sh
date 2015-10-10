if [ -z "$2" ]; then
	grep --include=*.cpp --include=*.h -Hnr $1 ../../src/
else
	grep --include=*.cpp --include=*.h -Hnr $1 ../../src/ > $2
fi
