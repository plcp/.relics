USER=alice

all: reset
	echo "Clés renouvellées"

key:
	cat /dev/urandom | base64 | head -n 1 | tee key.b64

user:
	cat /dev/urandom | base64 | base64 | head -n 1 | tee /dev/tty > $(USER).passwd
	echo '[{"username":"$(USER)", "password":"'"`cat $(USER).passwd`"'"}]' > users.json

reset: key user
	echo done !

useradd: isroot
	useradd -m -l -N -g nobody -e `date -d "3 days" +"%Y-%m-%d"` $(USER)

chpasswd: isroot
	echo -n "alice:" > /tmp/alice
	cat alice.passwd | base64 -d | cat /tmp/alice - | chpasswd

userdel: isroot
	(grep alice /etc/passwd && userdel -rf $(USER)) || true

userfix: userdel useradd chpasswd
	echo done !

isroot:
	echo "root ?"
	[ "`id -u`" -eq "0" ]
	echo "yes"

mr_proper: userdel
	rm users.json $(USER).passwd key.b64
