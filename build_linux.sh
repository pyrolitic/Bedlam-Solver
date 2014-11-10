if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
else
	GLEW_LOCATION="/usr";
fi

if [ -z "$CXX" ]; then
	CXX=g++; #must support c++11
fi

OUT=solver
SOURCES="src/state_show_solution.cpp src/main.cpp src/app.cpp src/graphics/font_bitmap.cpp src/state_editing.cpp src/state_listing.cpp src/state_solving.cpp src/solver.cpp"
LIBS="-lGLEW -lGLU -lglut -lGL -lm"

INCLUDEDIRS="-I\"$GLEW_LOCATION/include\""

CXXFLAGS="-std=c++11 -Wuninitialized -pthread -g3 -O0 -Wfatal-errors -Wformat-security -Wformat=2"
LINKFLAGS="-L\"$GLEW_LOCATION/lib\""

$CXX $CXXFLAGS $INCLUDEDIRS $LINKFLAGS -o $OUT $SOURCES $LIBS
