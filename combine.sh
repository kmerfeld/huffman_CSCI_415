#!/bin/bash
touch compressed.mpc
cat compressed-*.mpc >> compressed.mpc
rm ./compressed-*.mpc
