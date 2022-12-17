CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include 
CXX_INCLUDE_DIRS+=C:\MinGW\include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_DIRS+=C:\MinGW\lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))

all: 
	$(CXX) source/part1/console.cpp -g -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) source/part1/http_server.cpp -g -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) source/sock_server.cpp -g -o sock_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

sock:
	$(CXX) source/sock_server/sock_server.cpp source/sock_server/sock_session.cpp -w -g -o sock_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

clean:
	rm -f http_server
	rm -f console.cgi
	rm -f sock_server