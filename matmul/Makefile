CXX := g++

CXX_FLAGS := -std=c++20 -O3 -march=native -mtune=native

SRCS := $(wildcard matmul*.cpp)
BASE := $(basename $(SRCS))

n ?= 128 256 512 1024 2048
t ?= 1073741824 # = 1<<30

%.test: %.cpp test.cpp matmul.hpp
	$(CXX) $(CXXFLAGS) $< test.cpp -o $@

%.bench: %.cpp bench.cpp matmul.hpp
	$(CXX) $(CXXFLAGS) $< bench.cpp -o $@

test: $(addsuffix .test, $(BASE))
	@set -e; \
	for exe in $^; do \
		echo "==> $$exe"; \
		./$$exe; \
	done

bench: $(addsuffix .bench, $(BASE))
	@set -e; \
	for exe in $^; do \
		echo "==> $$exe"; \
		for N in $(n); do \
			printf "n=%4d  " $$N; \
			./$$exe $$N $(t); \
		done \
	done

clean:
	rm -f *.test *.bench
