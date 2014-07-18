#GLEW_INSTALL="$HOME/C/glew_college"
#export LD_LIBRARY_PATH="$GLEW_INSTALL/lib"
valgrind --leak-check=full --show-reachable=yes --suppressions=nvidia.supp ./solver
