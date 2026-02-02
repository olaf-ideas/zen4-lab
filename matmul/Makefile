CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=znver4 -mtune=znver4 -I/usr/include/openblas -fopenmp
ASMFLAGS := -fno-omit-frame-pointer -fverbose-asm -g3
LDFLAGS := -lopenblas

CXXFLAGS += $(EXTRA_CXXFLAGS)

DBGFLAGS := -ggdb -fsanitize=undefined,address -Wall -Wshadow -pedantic

SRCS := $(wildcard matmul*.cpp)
BASE := $(basename $(SRCS))


BUILD := build

n_test ?= 1 2 3 4 12 32 64 512 666

n ?= 12 32 64 128 256 336 512 666 712 1024 2048 4096
t ?= 4294967296 # = 1<<32
l ?= 64

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.test: %.cpp test.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(DGBFLAGS) $< test.cpp -o $@ $(LDFLAGS)
$(BUILD)/%.bench: %.cpp bench.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $< bench.cpp -o $@ $(LDFLAGS)

$(BUILD)/%.s: %.cpp bench.cpp matmul.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(ASMFLAGS) -S $< -o $@ $(LDFLAGS)

test: $(addprefix $(BUILD)/,$(addsuffix .test,$(BASE)))
	@set -e; \
	for exe in $^; do \
		echo "==> $$exe"; \
		for N in $(n_test); do \
			printf "n=%4d  " $$N; \
			./$$exe $$N $(t) $(l); \
		done; \
	done

bench: $(addprefix $(BUILD)/,$(addsuffix .bench,$(BASE)))
	@set -e; \
	ulimit -l unlimited; \
	for exe in $^; do \
		echo "==> $$exe"; \
		for N in $(n); do \
			printf "n=%4d  " $$N; \
			sudo chrt -f 80 numactl --physcpubind 8 --membind=0 taskset -c 8 ./$$exe $$N $(t) $(l); \
		done; \
	done

clean:
	rm -rf $(BUILD)
