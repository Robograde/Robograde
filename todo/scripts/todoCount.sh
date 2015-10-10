if [ -z "$2" ]; then
	grep --include=*.cpp --include=*.h -Hnr $1 ../../src/ | wc -l
else
	grep --include=*.cpp --include=*.h -Hnr $1 ../../src/ | wc -l > $2
fi
