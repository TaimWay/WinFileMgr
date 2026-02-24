# ...
.PHONY: all help clean build run

all: help

help: 
	@printf "Usage: make [target]\n"
	@printf "\n"
	@printf "Targets:\n"
	@printf "	make help                           - Show this help message\n"
	@printf "	make clean                          - Clean up the project\n"
	@printf "	make build                          - Build the project\n"
	@printf "	make run                            - Run the project\n"

clean:
	rm -rf build

build:
	cmake -S . -B build -G "MinGW Makefiles"
	cmake --build build --config Release

run:
	./build/FileMgr.exe --console