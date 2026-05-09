mkdir build
cd build
cmake -DCMAKE_C_COMPILER=/usr/bin/clang \
      -DCMAKE_BUILD_TYPE=Debug \
      ..
cmake --build .
