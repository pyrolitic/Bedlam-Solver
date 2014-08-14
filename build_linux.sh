CXX=clang++  #replace with gcc or whatever you have; must support c++11
CXXFLAGS="-std=c++11"
OUT=solver
SOURCES="src/main.cpp src/app.cpp src/graphics/font_bitmap.cpp src/state_editing.cpp src/state_listing.cpp src/state_solving.cpp src/state_show_solution.cpp src/solver.cpp"
LIBS="-lGLEW -lGLU -lglut -lGL -lm -pthread"
GLEW_HEADERS="/usr/include/GL" #"$HOME/C/glew_college"
GLEW_LIBS="/usr/lib"

$CXX $CXXFLAGS $SOURCES $LIBS -o $OUT -I"$GLEW_HEADER" -L"$GLEW_LIBS" -g3 -O -ferror-limit=1
