#!/bin/bash
emcc -Oz -s INVOKE_RUN=0 -s MALLOC=emmalloc -s ABORTING_MALLOC=0 -s TOTAL_MEMORY=33554432 -o mode7.js mode7.c
