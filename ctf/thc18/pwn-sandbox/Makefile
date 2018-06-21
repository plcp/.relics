IMG = pwn-sandbox
CTN = sandkox

build:
	-git clone https://github.com/webpy/webpy.git webpy
	mkdir -p build/sandkox
	cp -r webpy/web build
	cp src/server.py flag build
	cp src/sandkox/sandkox-preload.so build/sandkox
	docker build -t $(IMG) .
	-rm -rf build

export: build
	mkdir -p export
	cp src/sandbox.c export/

up: build
	docker run -d -p 9462:8080 --name $(CTN) --privileged=true $(IMG)

down:
	-docker rm -f $(CTN)

logs:
	docker logs -f $(CTN)

clean: down
	-docker rmi $(IMG)
	rm -rf export webpy

.PHONY: build export up down logs clean
