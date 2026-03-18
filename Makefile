all: build


test:
	build/Game_test

build_dir:
	mkdir -p build

build: build_dir
	cd build && \
	cmake ".." -DCMAKE_BUILD_TYPE=Release && \
	cmake --build .