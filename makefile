CXX=g++
CXXFLAGS=-Wall -g

SERVER_TARGET=udp_server
CLIENT_TARGET=udp_client

SERVER_SOURCES=udp_server.cpp server/server.cpp server/reliable_connection.cpp thread_pool/thread_pool.cpp
CLIENT_SOURCES=udp_client.cpp

SERVER_OBJECTS=$(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.cpp=.o)

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $(SERVER_OBJECTS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_TARGET) $(CLIENT_TARGET)

.PHONY: all clean