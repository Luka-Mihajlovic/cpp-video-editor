# https://stackoverflow.com/questions/31421616/c-creating-static-library-and-linking-using-a-makefile
CXX = g++
EXECNAME = runme
LIBRARY = libFilmMaster2000.a
OBJECTS = main.o libFilmMaster2000.o
SAMPLE_INPUT = test.bin
SAMPLE_OUTPUT = testEditedFile.bin

all: $(LIBRARY) $(EXECNAME)

$(LIBRARY): libFilmMaster2000.o
	ar rcs $@ $^

$(EXECNAME): main.o $(LIBRARY)
	$(CXX) -o $@ $^ -Wall -Wextra -O3

%.o: %.cpp
	$(CXX) -c $< -o $@ -Wall -Wextra -O3

test: all
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) reverse
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -M reverse
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -S reverse

	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) swap_channel 1,2
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -M swap_channel 1,2
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -S swap_channel 1,2

	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) clip_channel 1 [10,200]
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -M clip_channel 1 [10,200]
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -S clip_channel 1 [10,200]

	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) scale_channel 1 1.5
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -M scale_channel 1 1.5
	./$(EXECNAME) $(SAMPLE_INPUT) $(SAMPLE_OUTPUT) -S scale_channel 1 1.5

clean:
	rm -f *.o $(EXECNAME) $(LIBRARY) $(SAMPLE_OUTPUT)
