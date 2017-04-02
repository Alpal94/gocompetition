#!/bin/bash

gcc -o connect mongo.c $(pkg-config --cflags --libs libmongoc-1.0)
