GNU_HOST="i686-w64-mingw32-"
CXX="$GNU_HOST""g++"  #replace with gcc or whatever you have; must support c++11
CXXFLAGS="-std=c++11 -DM_PI=3.14159265358979323846"
OUT=win32/bundle/solver.exe
SOURCES="src/main.cpp src/app.cpp src/font_bitmap.cpp src/state_editing.cpp src/state_listing.cpp src/state_solving.cpp src/state_show_solution.cpp" #"src/main.cpp src/app.cpp src/solver.cpp src/font_bitmap.cpp src/piece_editing.cpp"
LIBS="-lglew32 -lfreeglut -lglu32 -lopengl32 -lwinmm -lgdi32 -lm -pthread"

HEADER_DIR="$(pwd)/win32/include"
LIB_DIR="$(pwd)/win32/lib"

$CXX $CXXFLAGS $SOURCES -o $OUT -I"$HEADER_DIR" -L"$LIB_DIR" $LIBS -g3 -O
