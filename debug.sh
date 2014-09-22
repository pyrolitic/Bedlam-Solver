if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
fi
$1 ./solver
