IMG=thc/moche
IMG_SSH=thc/moche-ssh
CTN=moche
CTN_SSH=moche-ssh

USERNAME=chall

export:
	mkdir -p export
	$(MAKE) -C src/
	cp src/moche-client.py export/moche.py
	cp src/moche-server.py src/crypto.py src/moche.pcap export/

build/wrapper:
	mkdir -p build/
	gcc -O2 -o build/wrapper src/wrapper.c

build: build/wrapper
	docker build -t $(IMG) -f Dockerfile-moche .
	docker build -t $(IMG_SSH) -f Dockerfile-ssh .

run: build
	docker run -d --name $(CTN_SSH) $(IMG_SSH) -ddd
	echo "Run src/run.sh to start the challenge."

logs:
	docker logs -f $(CTN_SSH)

stop:
	-docker rm -f $(CTN_SSH)

clean: stop
	rm build/ -r

clean-all: clean
	-docker rmi $(IMG_SSH) $(IMG)
	rm -r export/

prod: build
	cp src/run.sh /root/run-moche.sh
	chmod +x /root/run-moche.sh
	cp build/wrapper /usr/local/bin/run-moche
	chmod 755 /usr/local/bin/run-moche
	useradd -s /usr/local/bin/run-moche $(USERNAME)
	chown root:root /usr/local/bin/run-moche
	chmod +s /usr/local/bin/run-moche
	echo "$(USERNAME):$(USERNAME)" | chpasswd
	passwd -n 10000 $(USERNAME)

clean-prod:
	userdel -f $(USERNAME)

.PHONY: export build run logs stop clean clean-all prod clean-prod
