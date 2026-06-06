CXX=g++
CXXFLAGS=-std=c++17 -pthread -g

SRC=\
src/hashmap.cpp \
src/storage.cpp \
src/shared_prefs.cpp \
src/async.cpp \
examples/SharedPrefHelper.cpp

OUT=test

all:
	$(CXX) $(CXXFLAGS) $(SRC) examples/main2.cpp -o $(OUT)

leak-asan:
	$(CXX) $(CXXFLAGS) -fsanitize=address $(SRC) examples/main2.cpp -o $(OUT)
	./$(OUT)

leak-valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(OUT)

run:
	./$(OUT)

clean:
	rm -f $(OUT)