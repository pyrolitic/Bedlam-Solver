#custom definitions here
if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
else
	GLEW_LOCATION="/usr";
fi

if [ -z "$CXX" ]; then
	CXX=g++; #must support c++11
fi

OUT=solver
LIBS="-lGLEW -lGLU -lglut -lGL -lm"

INCLUDEDIRS="-I\"$GLEW_LOCATION/include\""

CXXFLAGS="-std=c++11"
COMPILECXXFLAGS="\"$CXXFLAGS\" -Wuninitialized -pthread -g3 -O0 -Wfatal-errors -Wformat-security -Wformat=2"
LINKFLAGS="-L\"$GLEW_LOCATION/lib\""

#-- script after this --
OLD_IFS="$IFS"; 
IFS=$'\n'; #newline as separator in for loops

dir=$(readlink -e $0 | xargs dirname);
echo "working in $dir";
pushd "$dir";

case "$1" in
	"clean")
		rm -rf "$dir/.build";
		;;
	"")
		search_match=".*\\.cpp";
		search_path=".";
		if [ -d "$dir/src" ]; then
			echo "there is a ./src dir, limiting search to that";
			search_path="./src";
		fi

		files=$(find $search_path -type f -regex $search_match);

		tmp_file=$(tempfile);
		mkdir -p "$dir/.build";
		for rel_path in $files; do
			echo "looking at $rel_path";
			file_name=$(basename $rel_path);
			file_dir=$(dirname $rel_path);

			echo "in dir $file_dir";
			mkdir -p "$dir/.build/$file_dir";

			$CXX $CXXFLAGS $INCLUDEDIRS -E "$rel_path" > "$tmp_file";
			cur_hash=$(sha1sum $tmp_file | cut -d ' ' -f 1);
			echo "hash of expanded: $cur_hash";

			if [ -f "$dir/.build/$rel_path" ]; then
				old_hash=$(sha1sum "$dir/.build/$rel_path" | cut -d ' ' -f 1);
				echo "file has extended version, with hash $old_hash";
				if [ "$cur_hash" == "$old_hash" ]; then
					echo "match";
				else
					echo "different, rebuilding";
					$CXX $COMPILECXXFLAGS $INCLUDEDIRS -E "$rel_path" > "$tmp_file";
					cp "$tmp_file" "$dir/.build/$rel_path";
				fi
			else
				echo "no expanded version existed already";
				cp "$tmp_file" "$dir/.build/$rel_path";
			fi
		done
		rm "$tmp_file";
		;;
	*)
		echo "invlid argument";
		;; 
esac

popd
IFS="$OLD_IFS";