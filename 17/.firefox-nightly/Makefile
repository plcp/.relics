all:
	./setup.sh

crontab: crontab-script crontab-install crontab-cleanup
	crontab -l

crontab-script:
	echo "#!/bin/bash\nyes|./setup.sh &> /dev/null" > crontab.sh
	chmod +x ./crontab.sh

crontab-install:
	crontab -l > ./crontab.old
	echo "\n0 10 * * * $(shell pwd)/crontab.sh" | cat crontab.old - | sort -u > ./crontab.new
	crontab ./crontab.new

crontab-cleanup:
	rm crontab.old crontab.new
