CXX = clang++
CFLAGS = -std=c++20 -c -Wall -Wextra -Wold-style-cast -Werror -Iinclude

SRC_DIR = src/
BUILD_DIR = build/
TARGET = sharky

SRCS = main.cpp bitmap.cpp
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -std=c++20 $^ -o $@

$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
	rm $(TARGET)

.PHONY = all clean
