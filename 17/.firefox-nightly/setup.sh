#!/bin/bash

function which(){ /bin/sh -c "which $@" ; }

run="$(mktemp -u XXXXXXXX)"
dir="$(mktemp -d "/tmp/firefox-nightly.XXXXXXXXXXXXXXXX")"
function mktemp(){ $(which mktemp) "$@" -p "$dir" "$run.XXXXXXXX"; }
function curl(){ $(which curl) -H 'Cache-Control: no-cache' "$@?$($(which mktemp) -u XXXXXXXX)" ; }

function cleanup(){ rm -rf "$dir" ; }
trap cleanup EXIT

url="https://archive.mozilla.org/pub/firefox/nightly/latest-mozilla-central"
ext="tar.bz2"
lang="en-US"
build="linux-x86_64"

target="$HOME/.firefox-nightly"
regex="firefox-[0-9]*.[0-9a-zA-Z]*.$lang.$build.$ext"
fname="$(curl "$url/" 2> /dev/null |egrep -o "$regex"| head -n 1)"
short="$(sed "s/.$lang.$build.$ext//g" <<< "$fname")"

grep -q "$fname" "$target/releases" &> /dev/null && \
	read -p "Nightly $short already installed, press enter to cancel. " && \
	exit

sums="$(mktemp)"
file="$(mktemp)"
sha256="$(mktemp)"
armory="$(mktemp)"

curl -# "$url/$fname" > "$file"
file "$file" | grep -q "^$file: bzip2 compressed data"		|| \
	read -p "Unexpected file format, press ^D to cancel. " 	|| \
	exit

curl "$url/$(echo $fname|sed "s/.$ext$/.checksums/")" 2> /dev/null > "$sums"
egrep "sha256 [0-9]* $fname$" < "$sums"|sed "s%^\([^ ]*\) .*$%\1 $file%g" > "$sha256"
(sha256sum -c "$sha256" |& sed "s%$file%Checksums%") || exit

report="$(mktemp)"
curl "$url/$(echo $fname|sed "s/.$ext$/.checksums.asc/")" 2> /dev/null > "$armory"
timeout 10 gpg --keyserver pgp.mit.edu --recv-keys 14F26682D0916CDD81E37B6D61B7B526D98F0353 &> /dev/null
gpg --verify "$armory" "$sums" &> "$report"

grep "Good" "$report" 							|| \
	read -p "No valid gpg signature found, press ^D to cancel. " 	|| \
	(cat "$report" ; false) || exit

tar -C "$dir" -xf "$file" \
	--checkpoint-action="ttyout=\rExtracting (%ds): %T (out of 142Mib) %*"

target_dir="$target/firefox/$short"
test -d "$target_dir" && \
	read -p "!! Warning: $target_dir already exists !!
$(:	        )It will be overwritten, press enter to cancel. " && \
	exit

mkdir -p "$target/firefox" 2> /dev/null
rm -rf "$target_dir" 2> /dev/null
mv -f "$dir/firefox" "$target_dir"

target_bin="$target/bin/firefox$(sed "s/firefox-\([0-9]*\).\([0-9a-zA-Z]*\)/\1\2/g" <<< "$short")"
mkdir -p "$target/bin" 2> /dev/null

cat > "$target_bin" <<- __EOF__
	#!/bin/bash
	echo 'Starting $short with profile "$target/profile"...'
	exec "$target_dir/firefox" --profile "$target/profile" --new-instance --no-remote --setDefaultBrowser
__EOF__
chmod +x "$target_bin"

echo "$fname" > "$target/releases"
mkdir -p "$target/profile" 2> /dev/null

grep -q "$target/bin" "$HOME/.bashrc" || \
	read -p "Press ^D to add \`export PATH=\"\$PATH:$target/bin\"\` to \"$HOME/.bashrc\". " || \
	echo "export PATH=\"\$PATH:$target/bin\"" >> "$HOME/.bashrc"

echo -e "\nNightly $short installed."
