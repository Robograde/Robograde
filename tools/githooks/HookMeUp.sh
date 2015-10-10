#!/bin/bash
shopt -s extglob
cd $(git rev-parse --show-toplevel)/tools/githooks 
cp -r !(*.txt|*.sh) $(git rev-parse --show-toplevel)/.git/hooks/ 
