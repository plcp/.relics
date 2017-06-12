
all:
	cd ./nnetcpp; cmake CMakeLists.txt
	cd ./nnetcpp; make -j
	cp ./nnetcpp/tests/sequencegeneration learn-keytoy

clean:
	cd ./nnetcpp; test ! -e Makefile || make clean
	find ./nnetcpp -iwholename '*cmake*' -not -name CMakeLists.txt -delete
	find ./nnetcpp -name 'Makefile' -delete

mr_proper: clean
	rm learn-keytoy
