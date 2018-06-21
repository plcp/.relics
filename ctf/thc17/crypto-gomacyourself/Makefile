IMG=crypto-gomacyourself
CTN=gomacyourself

build:
	docker build -t $(IMG) .

export:
	mkdir -p export
	cp src/server.py src/client.py export/
	cp src/crypto-client.py export/crypto.py

up: build
	docker run -d -t -p 1058:1058 --name $(CTN) $(IMG)

down:
	-docker rm -f $(CTN)

logs:
	docker logs -f $(CTN)

clean: down
	-docker rmi $(IMG)
	rm -rf export

.PHONY: build export up down logs clean
