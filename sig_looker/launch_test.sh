#!/bin/bash


if [ "$#" -eq "1" ]
then
    LD_LIBRARY_PATH=.:../sym_resolv:../dbg_print $1
fi
