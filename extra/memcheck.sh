#!/bin/sh
valgrind --memcheck:leak-check=full --suppressions=../extra/valgrind.supp ./abyss
