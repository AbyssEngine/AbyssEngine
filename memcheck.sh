#!/bin/sh
valgrind --memcheck:leak-check=full --suppressions=../valgrind.supp ./abyss
