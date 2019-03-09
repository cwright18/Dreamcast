all: project

project: projectMain.cpp
	g++ projectMain.cpp -Wall -oproject -lX11

clean:
	rm -f project
	rm *.ppm
