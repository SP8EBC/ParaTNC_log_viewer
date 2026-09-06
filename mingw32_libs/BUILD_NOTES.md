# Prebuilt static libs for i686-w64-mingw32

These two static libraries let `paratnc_log_viewer` be cross-compiled for
32-bit Windows without a full Boost/fmt cross-build every time. They were
built once on this machine and are checked in because building them from
source takes several minutes and needs network access to fetch the
Boost/fmt source tarballs.

Headers are **not** vendored here: the mingw build reuses the system
`/usr/include/boost` (from `libboost1.83-dev`) and `/usr/include/fmt` (from
`libfmt-dev`) headers, since they are verified byte-identical to the
upstream 1.83.0 / 9.1.0 sources used below (`diff -rq` against the
downloaded tarballs came back empty). If those apt packages are ever
upgraded to a different Boost/fmt release, these `.a` files must be rebuilt
to match.

## Toolchain

- `i686-w64-mingw32-gcc/g++` (GCC) 13-win32 variant (Ubuntu package
  `g++-mingw-w64-i686-win32` 13.2.0-6ubuntu1+26.1)
- win32 threading model was used (not posix) so the resulting EXE does not
  need `libwinpthread-1.dll` at runtime.

## fmt 9.1.0 -> libfmt.a

Matches the version of `libfmt-dev` installed on this system (9.1.0+ds1-2).

```sh
curl -sSL -o fmt-9.1.0.tar.gz https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz
tar xzf fmt-9.1.0.tar.gz && cd fmt-9.1.0 && mkdir build-mingw32 && cd build-mingw32

cat > ../../toolchain-i686-w64-mingw32.cmake <<'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain-i686-w64-mingw32.cmake \
  -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_DOC=OFF -DFMT_INSTALL=ON \
  -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_INSTALL_PREFIX=<install-dir>
make -j$(nproc) && make install
# -> <install-dir>/lib/libfmt.a
```

## Boost 1.83.0 -> libboost_program_options.a

Matches `libboost1.83-dev` / `libboost-program-options1.83-dev` installed on
this system.

```sh
curl -sSL -o boost_1_83_0.tar.gz https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.gz
tar xzf boost_1_83_0.tar.gz && cd boost_1_83_0
./bootstrap.sh --with-libraries=program_options

cat > user-config.jam <<'EOF'
using gcc : mingw32 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;
EOF

./b2 --user-config=user-config.jam --with-program_options \
  toolset=gcc-mingw32 target-os=windows address-model=32 architecture=x86 \
  link=static runtime-link=static threading=multi variant=release \
  --prefix=<install-dir> install
# -> <install-dir>/lib/libboost_program_options.a
```

## Regenerating

If these ever need rebuilding (Boost/fmt version bump, ABI mismatch, etc.),
rerun the two command blocks above and copy the resulting `.a` files into
this directory.
