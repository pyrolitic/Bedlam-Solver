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

INCLUDEDIRS=("-I\"$GLEW_LOCATION/include\"");
LIBDIRS=("-L\"$GLEW_LOCATION/lib\"" );
LIBS=("-pthread" "-lGLEW" "-lGLU" "-lglut" "-lGL" "-lm");
CXXFLAGS=("-std=c++11"); #-E only

COMPILECXXFLAGS=("${CXXFLAGS[@]}" "-Wuninitialized" "-Wfatal-errors" "-Wformat-security" "-Wformat=2"); #-c only
LINKCXXFLAGX=("${COMPILECXXFLAGS[@]}" "-pthread" "-g3" "-O0"); #linking

#-- script after this --

OLD_IFS="$IFS"; 
IFS=$'\n'; #newline as separator in for loops

dir="$(cd "$(dirname "$0")" && pwd)";
echo "working in $dir";
cd "$dir";

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
		tmp_file="$dir/.build/__temp.cpp"; #temp file must have cpp suffix
		ln -s $(tempfile) "$tmp_file";

		relink=false;
		obj_list=( );
		for rel_path in $files; do
			echo "looking at $rel_path";
			file_dir=$(dirname $rel_path);
			file_name=$(basename $rel_path);

			hash_path=".build/$file_dir/${file_name}.hash";
			object_path=".build/$file_dir/${file_name}.o";
			obj_list+=("$object_path");

			mkdir -p ".build/$file_dir"; #doesn't hurt, needed to create a file in there

			expand_commnad=("$CXX" "${CXXFLAGS[@]}" -E "$rel_path");
			"${expand_commnad[@]}" > "$tmp_file";

			compile_command=("$CXX" -c "${COMPILECXXFLAGS[@]}" "${INCLUDEDIRS[@]}" "$tmp_file" -o "$object_path");

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
			echo "";

			if [ $rebuild == true ]; then
				relink=true;
				"${compile_command[@]}";
				if [ $? != "0" ]; then
					echo "compile failed";
					break;
				fi
				echo "$cur_hash" > "$hash_path";
			fi
		done

		if [[ ( $relink == true || ! -f "$BINARY_NAME" ) ]]; then
			link_command=("$CXX" "${LINKCXXFLAGX[@]}" -o "$BINARY_NAME" "${obj_list[@]}" "${LIBS[@]}");
			"${link_command[@]}";
			if [ $? != "0" ]; then
				echo "link failed";
				exit;
			else
				echo "successfully relinked";
			fi
		else
			echo "apprently no need to relink. not sure if this is right, though";
		fi

		rm $(readlink -e "$tmp_file"); #remove actual file
		rm "$tmp_file"; #remove local symlink
		;;
	*)
		echo "invlid argument";
		;; 
esac

IFS="$OLD_IFS";