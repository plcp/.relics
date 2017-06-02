.PHONY: after-mr_proper__ after-pyshell__ after-setup__ after-shell__ all before-mr_proper__ before-pyshell__ before-setup__ before-shell__ empty__ mr_proper pyshell setup shell list

BRANCH="develop"
COMMIT="develop" # specify a commit (sha1) here

all: setup shell mr_proper



shell: before-shell__ after-shell__
	python ./fuddly/fuddly_shell.py

shell2: before-shell__ after-shell__
	python2 ./fuddly/fuddly_shell.py

shell3: before-shell__ after-shell__
	python3 ./fuddly/fuddly_shell.py

nshell: after-setup__ before-shell__ after-shell__
	python ./fuddly/fuddly_shell.py

nshell2: after-setup__ before-shell__ after-shell__
	python2 ./fuddly/fuddly_shell.py

nshell3: after-setup__ before-shell__ after-shell__
	python3 ./fuddly/fuddly_shell.py

before-shell__: empty__
	# TODO: Add your recipe here if needed.

after-shell__:
	# TODO: Add your recipe here if needed.



pyshell: before-pyshell__ after-pyshell__

before-pyshell__:
	cp ../wrappers/pyshell.py ./fuddly/fuddly_pyshell.py

after-pyshell__:
	bash -c " :								\
		; source ./fuddly/venv3/bin/activate 				\
		; HOME='$(shell pwd)'						\
		; cd '$(shell pwd)/fuddly'					\
		; python -i fuddly_pyshell.py					\
		; true								\
	"
	rm ./fuddly/fuddly_pyshell.py



setup: before-setup__ after-setup__

before-setup__:
	bash -c "cd ../fuddly ; \
		git checkout $(BRANCH) ; \
		git checkout $(COMMIT) ;" 2> /dev/null
	cp -r ../fuddly ./fuddly
	bash -c "cd ../fuddly ; git reset --hard ; git checkout $(BRANCH)" 2> /dev/null

	mkdir -p ./fuddly_data/imported_data
	test -e  ./requirements.txt || touch  ./requirements.txt
	cat ../requirements.txt ./requirements.txt > ./fuddly/requirements.txt
	
	sed -i "s/sqlite.//g" ./fuddly/requirements.txt
	
	if [ -d ../venv2 ]							 \
	;	then cp -r ../venv2 ./fuddly/venv2				 \
	;	else virtualenv --system-site-packages -p python2 ./fuddly/venv2 \
	; fi 									 \
									
	if [ -d ../venv3 ]							 \
	;	then cp -r ../venv3 ./fuddly/venv3				 \
	;	else virtualenv --system-site-packages -p python3 ./fuddly/venv3 \
	; fi 									 \

	bash -c " :								 \
		; source ./fuddly/venv2/bin/activate 				 \
		; HOME='$(shell pwd)/fuddly/venv2'				 \
		; cd '$(shell pwd)/fuddly/venv2' 				 \
		; pip2 install -r ../requirements.txt				 \
	"

	bash -c " :								 \
		; source ./fuddly/venv3/bin/activate 				 \
		; HOME='$(shell pwd)/fuddly/venv3'				 \
		; cd '$(shell pwd)/fuddly/venv3' 				 \
		; pip3 install -r ../requirements.txt				 \
	"


after-setup__:
	cp ../fuddly/tools/fmkdb.py ./fuddly/tools/fmkdb_wrapped.py
	cp ../wrappers/fmkdb.py ./fuddly/tools/fmkdb.py
	chmod +x ./fuddly/tools/fmkdb.py
	
	cp ../fuddly/fuddly_shell.py ./fuddly/fuddly_shell_wrapped.py
	cp ../wrappers/fuddly_shell.py ./fuddly/fuddly_shell.py
	chmod +x ./fuddly/fuddly_shell.py



mr_proper: before-mr_proper__ after-mr_proper__
	rm -rf ./fuddly
	rm -rf ./fuddly_data
	test -s ./requirements.txt || \
		rm -f ./requirements.txt

before-mr_proper__:
	test ! -d ./fuddly/venv2 || \
		test -d ../venv2 || \
			cp -r ./fuddly/venv2 ../venv2
	test ! -d ./fuddly/venv3 || \
		test -d ../venv3 || \
			cp -r ./fuddly/venv3 ../venv3

after-mr_proper__: empty__
	# TODO: Add your recipe here if needed.



empty__:
	: # Empty recipe tag

help:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort -r | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | grep -v __ | xargs
