
all:
	rm -rf embedding.json
	cat ./flag | bash regen.sh
	bash genreadme.sh
	yes | bash solve.sh

clean:
	rm -rf workdir

mr_proper: clean
	rm -rf src
	rm README.md
