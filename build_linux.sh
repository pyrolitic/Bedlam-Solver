if [ ! -z "$GLEW_LOCATION" ]; then
	export LD_LIBRARY_PATH="$GLEW_LOCATION/lib:$LD_LIBRARY_PATH:";
fi

CXX=clang++  #replace with gcc or whatever you have; must support c++11
CXXFLAGS="-std=c++11 -Wall -Wsometimes-uninitialized -Wuninitialized"
OUT=solver
SOURCES="src/state_show_solution.cpp src/main.cpp src/app.cpp src/graphics/font_bitmap.cpp src/state_editing.cpp src/state_listing.cpp src/state_solving.cpp src/solver.cpp"
LIBS="-lGLEW -lGLU -lglut -lGL -lm -pthread"

$CXX $CXXFLAGS $SOURCES $LIBS -o $OUT -I"$GLEW_LOCATION/include/GL" -L"$GLEW_LOCATION/lib" -pthread -g3 -O0 -Wfatal-errors -Wformat-security -Wformat=2
