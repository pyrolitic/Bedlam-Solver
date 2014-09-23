if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
else
	GLEW_LOCATION="/usr";
fi

if [ -z "$CXX" ]; then
	CXX=g++; #must support c++11
fi

CXXFLAGS="-std=c++11 -Wuninitialized"
OUT=solver
SOURCES="src/state_show_solution.cpp src/main.cpp src/app.cpp src/graphics/font_bitmap.cpp src/state_editing.cpp src/state_listing.cpp src/state_solving.cpp src/solver.cpp"
LIBS="-lGLEW -lGLU -lglut -lGL -lm -pthread"

$CXX -I"$GLEW_LOCATION/include" -L"$GLEW_LOCATION/lib" $CXXFLAGS $SOURCES $LIBS -o $OUT -pthread -g3 -O0 -Wfatal-errors -Wformat-security -Wformat=2
