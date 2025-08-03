#!/bin/bash

set -xe

cc -o cmdcenter $(find . -type f -name '*.c') $(forge lib)
