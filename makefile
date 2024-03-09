CXX=g++

CXXFLAGS=-Wall -g -pthread

TARGET=udp_server 

SOURCES=udp_server.cpp server/server.cpp server/reliable_connection.cpp thread_pool/thread_pool.cpp

OBJECTS=$(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) 


.PHONY: all clean
