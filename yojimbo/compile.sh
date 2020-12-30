#!/bin/bash

INCLUDES=-I${1}\ -I${2} LDFLAGS=-L${3}\ -L${4} make config=release_x64
