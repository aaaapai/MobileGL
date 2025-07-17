cmake -S . -B build && \
cmake --build build -j${nproc} && \
ctest --test-dir build
