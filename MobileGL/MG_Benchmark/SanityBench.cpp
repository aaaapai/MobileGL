// MobileGL - MobileGL/MG_Benchmark/SanityBench.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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