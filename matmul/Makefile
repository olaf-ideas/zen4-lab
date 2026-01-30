CXX := g++

CXXFLAGS := -std=c++20 -O1 -march=znver4 -mtune=znver4 -g -I/usr/include/openblas -fopenmp -lopenblas
ASMFLAGS := -fno-omit-frame-pointer -fverbose-asm -g3

SRCS := $(wildcard matmul*.cpp)
BASE := $(basename $(SRCS))

BUILD := build

n ?= 128 256 512 1024 2048
t ?= 4294967296 # = 1<<32

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.test: %.cpp test.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $< test.cpp -o $@ -fsanitize=undefined,address -Wall -Wshadow -pedantic
$(BUILD)/%.bench: %.cpp bench.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $< bench.cpp -o $@

$(BUILD)/%.s: %.cpp bench.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(ASMFLAGS) -S $< -o $@

test: $(addprefix $(BUILD)/,$(addsuffix .test,$(BASE)))
	@set -e; \
	for exe in $^; do \
		echo "==> $$exe"; \
		./$$exe; \
	done

bench: $(addprefix $(BUILD)/,$(addsuffix .bench,$(BASE)))
	@set -e; \
	ulimit -l unlimited; \
	for exe in $^; do \
		echo "==> $$exe"; \
		for N in $(n); do \
			printf "n=%4d  " $$N; \
			sudo chrt -f 80 numactl --physcpubind 8 --membind=0 taskset -c 8 ./$$exe $$N $(t); \
		done; \
	done

clean:
	rm -rf $(BUILD)
