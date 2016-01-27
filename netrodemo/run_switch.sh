#!/bin/bash

PROG="demo"

source ../debug/env.sh
$P4C_BM_SCRIPT $PROG.p4 --json $PROG.json
sudo PYTHONPATH=$PYTHONPATH:$BMV2_PATH/mininet/ python topo.py \
    --behavioral-exe $BMV2_PATH/targets/simple_switch/simple_switch \
    --json $PROG.json \
    --cli $CLI_PATH
