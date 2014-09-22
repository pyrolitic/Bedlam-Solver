if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
fi

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --suppressions=nvidia.supp ./solver
