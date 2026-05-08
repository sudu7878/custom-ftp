#Makefile ver0.0.1

BUILD = builds/TestBuilds

	
all:
	g++ -Iheaders src/CommMod.cpp src/ConnMod.cpp -o ftp


