#!/bin/bash
if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
else
	GLEW_LOCATION="/usr";
fi

if [ -z "$CXX" ]; then
	CXX=g++; #must support c++11
fi

BINARY_NAME="solver";

INCLUDEDIRS=("-I$GLEW_LOCATION/include");
LIBDIRS=("-L$GLEW_LOCATION/lib" );
LIBS=("-pthread" "-lGLEW" "-lGLU" "-lglut" "-lGL" "-lm");

CXXFLAGS=("-std=c++11" "${INCLUDEDIRS[@]}"); #-E only
COMPILECXXFLAGS=("${CXXFLAGS[@]}" "-Wuninitialized" "-Wfatal-errors" "-Wformat-security" "-Wformat=2"); #-c only
LINKCXXFLAGS=("${LIBDIRS[@]}" "${COMPILECXXFLAGS[@]}" "${LIBS[@]}" "-g3" "-O0"); #linking

#-- script after this --

OLD_IFS="$IFS"; 
IFS=$'\n'; #newline as separator in for loops

dir="$(cd "$(dirname "$0")" && pwd)";
echo "working in $dir";
cd "$dir";

tmp_file="$dir/.build/__temp.cpp"; #temp file must have cpp suffix
ln -s $(tempfile) "$tmp_file";

cleanup() {
	echo "cleanup";
	if [ -f "$tmp_file" ]; then
		rm $(readlink -e "$tmp_file"); #remove actual file
		rm "$tmp_file"; #remove local symlink
	fi
}

trap cleanup SIGINT SIGTERM;

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

		mkdir -p "$dir/.build";
		files=$(find $search_path -type f -regex $search_match | grep -v \.build | grep -v "test");

		relink=false;
		obj_list=( );
		for rel_path in $files; do
			rel_path=$(echo "$rel_path" | cut -c 3-); #remove ./
			echo -n "$rel_path.. ";
			file_dir=$(dirname $rel_path);
			file_name=$(basename $rel_path);

			hash_path=".build/$file_dir/${file_name}.hash";
			object_path=".build/$file_dir/${file_name}.o";
			obj_list+=("$object_path");

			mkdir -p ".build/$file_dir";

			expand_commnad=("$CXX" "${CXXFLAGS[@]}" -E "$rel_path");
			"${expand_commnad[@]}" > "$tmp_file";
			exit;

			#compile_command=("$CXX" -c "${COMPILECXXFLAGS[@]}" "$tmp_file" -o "$object_path");
			compile_command=("$CXX" -c "${COMPILECXXFLAGS[@]}" "$rel_path" -o "$object_path");
			echo "${compile_command[@]}";
			exit;

			cur_hash=$(sha1sum "$tmp_file" | cut -d ' ' -f 1);

			rebuild=false;
			if [[ ( -f "$hash_path" && -f "$object_path" ) ]]; then
				old_hash=$(cat "$hash_path");
				if [ "$cur_hash" == "$old_hash" ]; then
					echo "unchanged";
				else
					echo "changed";
					rebuild=true;
				fi
			else
				echo "no old version exists";
				rebuild=true;
			fi

			if [ $rebuild == true ]; then
				echo "rebuilding $rel_path";
				relink=true;
				"${compile_command[@]}";
				if [ $? != "0" ]; then
					echo "compile failed";
					break;
				fi
				echo "$cur_hash" > "$hash_path";
			fi

			#echo "";
		done

		if [[ ( $relink == true || ! -f "$BINARY_NAME" ) ]]; then
			echo -n "linking.. ";
			link_command=("$CXX" "${LINKCXXFLAGS[@]}" -o "$BINARY_NAME" "${obj_list[@]}");
			"${link_command[@]}";
			if [ $? != "0" ]; then
				echo "failed";
			else
				echo "successful";
			fi
		else
			echo "apprently no need to relink. not sure if this is right, though";
		fi

		cleanup;
		;;
	*)
		echo "invlid argument";
		;; 
esac

IFS="$OLD_IFS";
