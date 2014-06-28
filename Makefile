CXX?=g++-4.7
CXXFLAGS=-Wall -pthread -std=c++11
LDFLAGS=-pthread -lpq

test: test.cpp
	$(CXX) -o test $(CXXFLAGS) $(LDFLAGS) test.cpp
