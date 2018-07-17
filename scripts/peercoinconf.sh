#!/bin/bash -ev

mkdir -p ~/.peercoin
echo "rpcuser=username" >>~/.peercoin/peercoin.conf
echo "rpcpassword=`head -c 32 /dev/urandom | base64`" >>~/.peercoin/peercoin.conf
echo "enforcecheckpoint=0" >>~/.peercoin/peercoin.conf
