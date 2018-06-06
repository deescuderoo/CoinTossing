#!/usr/bin/env bash

# {1} - party start idx
# {2} - party end idx
# {3} - number of parties
# {4} - parties file path
# {5} - size of data
# {6} - internal iterations number


for party_idx in `seq ${1} 1 ${2}`;
do
    ./cmake-build-debug/CoinTossing -partyID ${party_idx} -partiesNumber ${3} -partiesFile ${4} -D ${5} -internalIterationsNumber ${6} &
    echo "running ${party_idx}"
done