#!/bin/bash

. ~/.bashrc

./run_profile_ens.sh > ens.out 2> ens.err & savepid
