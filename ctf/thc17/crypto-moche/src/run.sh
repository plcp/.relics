#!/bin/bash
# Initialize a container and display mapped ports


clean() {
    docker rm -f "$CTN" &>/dev/null
    exit 0
}

run() {
    readonly USED=("$(netstat -l --udp | awk '/^udp/ { split($4, addr, ":"); print addr[2] }')")

    while true; do
        P1="$(($RANDOM + 10000))" # between 10000 and (10000 + 32767)
        P2="$(($P1 + 1))"

        [[ ! "$USED" =~ (^|[[:space:]])$P1($|[[:space:]]) ]] && \
        [[ ! "$USED" =~ (^|[[:space:]])$P2($|[[:space:]]) ]] && \
        break
    done

    readonly IMG=thc/moche
    readonly CTN="$(docker run -p "$P1-$P2:48000-48001/udp" --link moche-ssh:ssh --network=srv_moche_nw -d "$IMG")"

    readonly CTRL_PORT="$P1"
    readonly COMM_PORT="$P2"

    cat <<EOF
The server is running:
 - port 48000 is mapped on $CTRL_PORT
 - port 48001 is mapped on $COMM_PORT

Press enter to close the connection.
EOF

    # Wait for the connection to be closed
    read
    clean
}

# Catch ctrl-c and call clean()
trap clean INT
run
