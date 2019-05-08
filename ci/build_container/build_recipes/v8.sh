#!/bin/bash

set -e

# Get wasm-c-api.

COMMIT=0705a10690d42ff312102392a7f6eeda56985182  # Tue Apr 16 14:05:11 2019 +0200
SHA256=dc4ea492e4a45ea8aa482091585710a44719bc60a60b958fe5aa4e22fa8887ba

curl https://github.com/WebAssembly/wasm-c-api/archive/"$COMMIT".tar.gz -sLo wasm-c-api-"$COMMIT".tar.gz \
  && echo "$SHA256" wasm-c-api-"$COMMIT".tar.gz | sha256sum --check
tar xf wasm-c-api-"$COMMIT".tar.gz
cd wasm-c-api-"$COMMIT"

# Build v8 inside v8 subdirectory to match wasm-c-api's Makefile.

mkdir v8
cd v8

# Get depot_tools.

git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH="$PATH:$PWD/depot_tools"

# Get v8.

VERSION=7.4.288.26  # match wasm-c-api (branch-heads/7.4)

fetch v8
cd v8
git checkout "$VERSION"
gclient sync

# Patch v8 for wasm-c-api.

patch -p1 < ../../patch/0001-BUILD.gn-add-wasm-v8-lowlevel.patch
cp -p ../../src/wasm-v8-lowlevel.cc src/wasm-v8-lowlevel.cc
cp -p ../../src/wasm-v8-lowlevel.hh include/wasm-v8-lowlevel.hh

# Build v8 static library.

tools/dev/v8gen.py x64.release -- v8_monolithic=true v8_use_external_startup_data=false v8_enable_i18n_support=false v8_enable_gdbjit=false use_custom_libcxx=false
ninja -v -C out.gn/x64.release v8_monolith

# Install v8.

mkdir -p "$THIRDPARTY_BUILD/include/v8/libplatform"
cp -p include/v8*.h "$THIRDPARTY_BUILD/include/v8/"
cp -p include/libplatform/*.h "$THIRDPARTY_BUILD/include/v8/libplatform/"
cp -p out.gn/x64.release/obj/libv8_monolith.a "$THIRDPARTY_BUILD/lib/"

cd ../..

# Patch wasm-c-api.

cat <<EOF | patch -p1
--- a/src/wasm-v8.cc
+++ b/src/wasm-v8.cc
@@ -296,7 +296,7 @@ auto Engine::make(own<Config*>&& config) -> own<Engine*> {
   v8::internal::FLAG_experimental_wasm_bigint = true;
   v8::internal::FLAG_experimental_wasm_mv = true;
   // v8::internal::FLAG_experimental_wasm_anyref = true;
-  // v8::internal::FLAG_experimental_wasm_bulk_memory = true;
+  v8::internal::FLAG_experimental_wasm_bulk_memory = true;
   // v8::V8::SetFlagsFromCommandLine(&argc, const_cast<char**>(argv), false);
   auto engine = new(std::nothrow) EngineImpl;
   if (!engine) return own<Engine*>();
EOF

# Build wasm-c-api.

# TODO(PiotrSikora): respect CC/CXX/CFLAGS/CXXFLAGS/LDFLAGS upstream.

# Disable sanitizers.
sed -i"" -E "s/ -fsanitize[^\n]+//" Makefile

make wasm

# Install wasm-c-api.

mkdir -p "$THIRDPARTY_BUILD/include/wasm-c-api"
cp -p include/wasm.hh "$THIRDPARTY_BUILD/include/wasm-c-api/"
cp -p src/wasm-bin.hh "$THIRDPARTY_BUILD/include/wasm-c-api/"
ar -r -c -s "$THIRDPARTY_BUILD/lib/libwasm.a" out/wasm-bin.o out/wasm-v8.o
