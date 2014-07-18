
files=$(find src/unit_tests -type f -name "*.cpp")

for f in $files
do
	echo $f
	filename=$(basename "$f")
	stem="${filename%.*}"
	clang++ -std=c++11 -g3 -O -o "tests/$stem" "$f" -Wall
done

