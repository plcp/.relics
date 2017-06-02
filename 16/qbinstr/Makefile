cfiles=qbinstr.c;args.c
hfiles=define.h;globals.h;tools.h;slots_offset.h;ins_t.h;th_data_t.h;tracer.h;instrument.h;callbacks.h;args.h;args_info.h;probe.h;hybrid.h;instrument_hybrid.h
files=$(cfiles);$(hfiles)
wfiles=$(subst ;, ,$(files))

all: build

build: update prepare patch
	cd dynamorio && cmake CMakeLists.txt
	$(MAKE) -j -C dynamorio all
	cp dynamorio/api/bin/libqbinstr.so ./libqbinstr.so

debug: update prepare patch
	cd dynamorio && cmake -DDEBUG=on CMakeLists.txt
	$(MAKE) -j -C dynamorio all
	cp dynamorio/api/bin/libqbinstr.so ./libqbinstr.so

update:
	(git pull && git submodule update --init --recursive) || (echo "Skip submodule update ? (^D to Cancel)" && read || exit 1)

prepare:
	tar -cf source.tar $(wfiles)
	cp -f source.tar dynamorio/api/samples/
	cd dynamorio/api/samples && tar -xvf source.tar

patch:
	cd dynamorio && git checkout HEAD -- api/samples/CMakeLists.txt
	echo "add_sample_client(qbinstr \"$(cfiles)\" \"drmgr;drreg;drutil\")" \
		>> dynamorio/api/samples/CMakeLists.txt

gengetopt: gengetopt-wipe
	gengetopt -i qbinstr.ggo -f args_parser -a args_info_t -F args

gengetopt-wipe:
	rm -f args.c args.h

clean:
	(cd dynamorio/api/samples &> /dev/null && rm -f source.tar $(wfiles) &> /dev/null) || true

mr_proper: clean
	rm -rf dynamorio
	rm -f libqbinstr.so
	rm -f qbinstr_pid_*_th_*.log

