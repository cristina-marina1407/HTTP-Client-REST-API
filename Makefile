CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

SRC = source/buffer.cpp source/client.cpp source/commands.cpp source/helper.cpp source/requests.cpp
OBJ = $(SRC:.cpp=.o)
DEPS = buffer.hpp client.hpp commands.hpp helper.hpp requests.hpp json.hpp json_fwd.hpp

EXEC = client

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)
