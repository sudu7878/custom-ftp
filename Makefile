#Makefile ver0.0.1

BUILD = builds/TestBuilds

	
all:
	g++ -Iheaders src/CommunMod.cpp src/api.cpp src/ClientMod.cpp src/ServerMod.cpp src/UserHandler.cpp -o ftp


	
