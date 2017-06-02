#!/usr/bin/env bash

# crawler config
target="ix.io"
prefix="ixio"
multip="1" # crawler oversampling (poll $multip times more)

# wipe terminal
tput clear

# target directory
echo "!! Saving files into $(pwd)/$prefix..."
mkdir -p "$prefix"
cd "$prefix"

# history of the paste downloaded
history=".history"
touch "$history"

# cosmetics
gt="$(tput bold)>$(tput sgr0)"

# query $target and retrieve pastes
total="0"
function refresh_base()
{
	# initialize variables
	last_total="0"
	history_content="$(cat "$history"|sort -r)"

	# foreach paste
	while read -r line ;
	do
		# (empty parser output, discard)
		[ "$line" == $'\n\n' ] && continue

		# retrieve the name of the paste, skip if already known
		read -r name < <(sed 's/^<\([^<]*\)<.*/\1/g' <<< "$line")
		[[ "$history_content" =~ "$name" ]] && continue

		# retrieve the filename & the date
		file="$(sed 's/^<.*< <\(.*\)< <.*<$/\1/g;s/\s*$//g;s_/_-_g;' <<< "$line")"
		date="$(
			date -d "$(
				sed 's/^<.*< <.*< <\(.*\)<$/\1/g;s/\s*$//g;' <<< "$line"
				)" +%s)"

		# split pastes into several dated directories
		ddir="$(date -d "@$date" +%Y%m%d)"

		# retrieve the paste content & checksum
		ppay="$(curl "$target/$name" 2> /dev/null)"
		ssum="$(echo -n "$ppay"|sha256sum|head -c 16)"

		# save the paste
		mkdir -p "$ddir"
		test -z "$file" && file="noname"
		final="$ddir/${name}_$(date -d "@$date" +%H%M%S)_${ssum}_$file"
		echo "$ppay" > "$final"

		# update history
		echo "$date $name" >> "$history"
		history_content="$history_content\n$date $name"

		# display cosmetics
		echo -n " | $gt $(tput bold)$(tput dim)$name$(tput sgr0;tput dim) downladed"
		size="$(wc -c "$final" | cut -f 1 -d ' ')"
		scol="$(tput setaf 2)$(tput dim)"
		if   [ "$size" -gt 262144 ];
		then scol="$(tput setaf 1)$(tput bold)"
		elif [ "$size" -gt 131072 ];
		then scol="$(tput setaf 3)$(tput bold)"
		elif [ "$size" -gt 65536 ];
		then scol="$(tput setaf 1)"
		elif [ "$size" -gt 32768 ];
		then scol="$(tput setaf 3)"
		elif [ "$size" -gt 16384 ];
		then scol="$(tput setaf 7)"
		elif [ "$size" -gt 8192 ];
		then scol="$(tput setaf 7)$(tput dim)"
		elif [ "$size" -gt 4096 ];
		then scol="$(tput setaf 3)$(tput dim)"
		elif [ "$size" -gt 2048 ];
		then scol="$(tput setaf 6)$(tput dim)"
		elif [ "$size" -gt 1024 ];
		then scol="$(tput setaf 2)"
		fi

		last_total="$(($last_total + $size))"
		read size < <(bc <<< "scale=3; $size / 1000")

		printf  "$(tput sgr0) ($scol%8skb$(tput sgr0)) " "$size"
		echo -e "$(tput sgr0;tput dim)-> $(tput setaf 5)$final$(tput sgr0)"

		# here below are the /user/ parser
	done < <(curl "$target/user/" 2> /dev/null | tr '[:space:]' ' ' | grep -ao '<div class="t"> *\([^ ]\+\) *<a href="/\1">\[r\]</a> *<a href="/\1/">\[h\]</a>[^@]*@[^<]*</div>' | sed -s 's_<div class.*</a> *<a href="/\([^/]*\)/">\[h\]</a>\s*\([^@]*\)@ \([^<]*\)</div>_<\1< <\2< <\3<_g' | sed -s "s_'_[:squote:]_g" | sed -s 's_"_[:dquote:]_g' )

	# skip cosmetics if no paste downloaded
	[ "$last_total" == "0" ] && return

	# display cosmetics
	read disp_total < <(bc <<<"scale=3; $last_total / 1000")
	echo " | $gt Total downloaded: $(tput bold)${disp_total}kb$(tput sgr0)"

	read disp_total < <(bc <<<"scale=3; $total / 1000")
	echo -n " $gt Total: $(tput setaf 6)${disp_total}"

	read disp_total < <(bc <<<"scale=3; $last_total / 1000")
	echo -n "+${disp_total}kb = $(tput sgr0)"

	new_total="$(($total + $last_total))"
	read disp_total < <(bc <<<"scale=3; $new_total / 1000")
	echo -n "$(tput bold)$(tput setaf 4)${disp_total}kb"

	echo "$(tput sgr0)"
	total="$new_total"
}

# wait a bit before querying again
function wait_average()
{

	# calculate the average rate of pastes creation
	sum="0"
	last=""
	count="1"
	while read stamp name ;
	do
		[ -z "$last" ] && last="$stamp" && continue

		dlt="$(($stamp - $last))"
		sum="$(($sum + $dlt))"
		last="$stamp"
		count="$(($count + 1))"

		# use up to the 64 last pastes for the stats
	done <<< "$(cat "$history"|sort|tail -n 64)"

	# display cosmetics
	tosleep="$(bc <<< "scale=4; $sum / $count / $multip")"
	echo " $gt $(tput setaf 3)Suspending for ${tosleep}s$(tput sgr0)..."
	echo -ne "\n\n\n\n"
	tput cuu 3
	
	echo -ne "\r  (maintain enter to exit, ^D to skip) $(tput el)"
	read -t "$tosleep" && exit 0
	echo -ne "\r $(tput el)"
	tput cuu 1
}

# cleanup terminal upon exiting
function cleanup()
{
	tput sgr0
}
trap cleanup EXIT

# dowload, wait, download, wait... ^D to terminate
while true;
do
	# display cosmetics
	echo -ne "\n$(tput bold;tput setaf 4)Refreshing...$(tput sgr0)\n $gt $(tput setaf 6)"
	date ; tput sgr0

	# download & wait
	refresh_base;
	wait_average;
done

