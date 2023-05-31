
all:
	g++ -o dockersave dockersave.cpp -lboost_filesystem -lboost_system -lboost_iostreams

clean:
	rm *.o dockersave
