#!/bin/bash

echo "🔧 Compiling with memory debugging..."
make clean
make CFLAGS="-Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -g -fsanitize=address"

echo "🚀 Running with AddressSanitizer..."
./build/fanorona

echo "✅ Memory verification complete"
