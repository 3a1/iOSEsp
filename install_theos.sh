#!/usr/bin/env bash

# Theos framework installed?
if [ -z "$THEOS" ]; then
    #
    #   We will install roothide Theos because it supports both rootless and roothide compilation.
    #   In this way we don't need create two different branches for roothide and rootless compilation.
    #
    bash -c "$(curl -fsSL https://raw.githubusercontent.com/roothide/theos/master/bin/install-theos)"
else
    echo "Theos is already installed at $THEOS"
fi
