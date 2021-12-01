# -*- MakeFile -*-

sysdb:	cMain.o cApp.o DBMS-Functions-Classes.o
	g++ -pipe cMain.o cApp.o DBMS-Functions-Classes.o `wx-config --libs` -o sysdb

cMain.o: cMain.cpp
	g++ -pipe -c cMain.cpp `wx-config --cxxflags`

cApp.o: cApp.cpp
	g++ -pipe -c cApp.cpp `wx-config --cxxflags`

DBMS-Functions-Classes.o: DBMS-Functions-Classes.cpp
	g++ -pipe -c DBMS-Functions-Classes.cpp `wx-config --cxxflags`

clean:
	rm *.o sysdb


