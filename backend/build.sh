#!/usr/bin/env bash

#
#   For executable we don't care if it's compiled for rootless or roothide.
#   Rootless version works same with roothide Dopamine.
#
make THEOS_PACKAGE_SCHEME=rootless
