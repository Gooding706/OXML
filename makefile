release:
	g++ main.cpp -I includes/ src/*.cpp -O3 -o builds/build
debug:
	g++ main.cpp -I includes/ src/*.cpp -g -o builds/build