// MobileGL - MobileGL/MG_Benchmark/SanityBench.cpp
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include <benchmark/benchmark.h>
#include <vector>
#include <cstring>

static void Sanity_VectorResize(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v;
        v.resize(state.range(0), 0);
        for (int i = 0; i < state.range(0); i += 16) {
            v[i] = i;
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
BENCHMARK(Sanity_VectorResize)->Arg(2 << 5)->Arg(2 << 10)->Arg(2 << 20);

static void Sanity_memcpy(benchmark::State& state) {
    char* src = new char[state.range(0)];
    char* dst = new char[state.range(0)];
    memset(src, 'x', state.range(0));
    for (auto _ : state)
        memcpy(dst, src, state.range(0));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
    delete[] src;
    delete[] dst;
}
BENCHMARK(Sanity_memcpy)->Range(8, 8 << 10);

BENCHMARK_MAIN();