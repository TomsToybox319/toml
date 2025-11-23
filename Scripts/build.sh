#!/bin/bash
set -e

if [ -d "external"]; then
else
    git clone https://github.com/TomsToybox319/testlib.git external
    cd external
    . Scripts\build.sh
    cd ..
fi

mkdir -p Build
cd Build

CompilerOptions="-o main"  # Output file will be named "main"
CompilerOptions+=" -Wall  -Werror "  # Turn on more compiler warnings and treat them as errors
CompilerOptions+=" -Weverything" # Warn all of the things!
CompilerOptions+=" -Wno-c++98-compat" # Don't need C++98 comptability
CompilerOptions+=" -Wno-padded" # Don't care about padding
CompilerOptions+=" -Wno-global-constructors" # We need global ctors
CompilerOptions+=" -Wno-exit-time-destructors" # We need exit-time dtors
CompilerOptions+=" -Wno-weak-vtables" # Disable warnings about vtables
CompilerOptions+=" -std=c++23"   # Compile with C++23 standard                                           
CompilerOptions+=" -I../Src/"    # Set include directory
CompilerOptions+=" -I../external/Src/"    # Set include directory
clang++  ../main.cpp ../external/Build/unittest.obj $CompilerOptions

cd ..
