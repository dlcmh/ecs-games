CXX      ?= c++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
APP      := build/rotating-square
SRC      := src/main.mm src/game.cpp
HDR      := src/ecs.hpp

$(APP): $(SRC) $(HDR)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -fobjc-arc -framework Cocoa $(SRC) -o $(APP)

run: $(APP)
	./$(APP)

clean:
	rm -rf build

.PHONY: run clean
