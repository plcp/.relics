#!/bin/sh

# Create diffie-hellman parameter set (forward secrecy)
function new_dh()
{
    openssl dhparam -out $1 1024
}

# Create for « $1 » a self-signed certificate « $2 » and key « $3 »
function new_ca()
{
    openssl req             \
        -newkey rsa:4096    \
        -nodes              \
        -x509               \
        -sha256             \
        -days 365           \
        -subj "/CN=$1"      \
        -out "$2"           \
        -keyout "$3"
}

# Create for « $1 » a Certificate Signing Request « $2 » and key « $3 »
function new_csr()
{
    openssl req             \
        -newkey rsa:2048    \
        -nodes              \
        -subj "/CN=$1"      \
        -out "$2"           \
        -keyout "$3"
}

# Use ca « $2 » and csr « $1 » to create « $4 » certificate
function new_cert()
{
    openssl x509            \
        -req                \
        -sha256             \
        -days 31            \
        -in "$1"            \
        -CA "$2"            \
        -CAkey "$2"         \
        -CAcreateserial     \
        -out "$3"
}

# Take « $1 » public and « $2 » private, merge them in « $3 »
function merge()
{
    cat "$2" "$1" > "$3"
    rm "$2"
}

# Populate the keystore
function populate()
{
    mkdir ca.d

    # Create server's Certificate Authority
    cd ca.d
    new_ca "ca_private" "ca_public.pem" "ca_private.key"
    merge "ca_public.pem" "ca_private.key" "ca_private.pem"
    cd ..

    # (an « evil » server, for testing purposes)
    cd ca.d
    new_ca "evil" "evil.pem" "evil.pem"
    cd ..


    mkdir frontends.d
    mkdir backends.d

    function add()
    {
        mkdir build
        cd build

        new_csr "$1" "$1.csr" "$1.key"
        new_cert "$1.csr" "../$3.pem" "$1_public.pem"
        merge "$1_public.pem" "$1.key" "$1_private.pem"

        mv "$1_private.pem" "../$2/"
        mv "$1_public.pem" "../$2/"

        cd ..
        rm -rf build
    }

    add server  .             ./ca.d/ca_private

    add irssi   frontends.d   ./ca.d/ca_private
    add poezio  frontends.d   ./ca.d/ca_private
    add irc     backends.d    ./ca.d/ca_private
    add xmpp    backends.d    ./ca.d/ca_private
    add matrix  backends.d    ./ca.d/ca_private

    new_dh param.dh.pem

    mkdir evil.d
    add discord evil.d        ./ca.d/evil
}
