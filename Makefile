all: build test


test:
	build/Game_test

build_dir:
	mkdir -p build

build: build_dir
	cd build && \
	cmake ".." -DCMAKE_BUILD_TYPE=Release && \
	cmake --build .

build_debug: build_dir
	cd build && \
	cmake ".." -DCMAKE_BUILD_TYPE=Debug && \
	cmake --build .
debug: build_debug
	cd build && \
	sudo lldb -- ./Game_test --gtest_filter=GameWithFixedParams.Snake3_BfsExploresReachableHeadPositions
