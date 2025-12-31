// MobileGL - MobileGL/MG_Benchmark/Buffer/BufferBench.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include <cstring>
#include <vector>
#include <benchmark/benchmark.h>

#include "MG_Impl/GLImpl/Buffer/GL_Buffer.h"
#include "MG_State/GLState/Core.h"

using namespace MobileGL;
using namespace MobileGL::MG_Impl::GLImpl;

constexpr GLuint BUFFER_COUNT = 32;
constexpr GLsizeiptr BUFFER_SIZE = 1024 * 1024;

static void BM_GenerateAndDeleteBuffers(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    GLuint n = static_cast<GLuint>(state.range(0));
    int repeat = static_cast<int>(state.range(1));
    std::vector<GLuint> buffers(n);

    for (auto _ : state) {
        for (int i = 0; i < repeat; i++) {
            GenBuffers(n, buffers.data());
            DeleteBuffers(n, buffers.data());
        }
    }

    state.SetItemsProcessed(state.iterations() * repeat * n);
    delete MG_State::pGLContext;
}

BENCHMARK(BM_GenerateAndDeleteBuffers)
    ->Args({128, 5})
    ->Args({1024, 10})
    ->Args({1536, 16})
    ->Args({2048, 32})
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime();

static void BM_CreateBufferObjectsAndBindBuffer(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    GLuint buffers[BUFFER_COUNT];
    GenBuffers(BUFFER_COUNT, buffers);

    for (auto _ : state) {
        for (GLuint i = 0; i < BUFFER_COUNT; i++) {
            GLenum target = (i % 2 == 0) ? GL_ARRAY_BUFFER : GL_UNIFORM_BUFFER;
            BindBuffer(target, buffers[i]);
        }
    }

    state.SetItemsProcessed(state.iterations() * BUFFER_COUNT);

    DeleteBuffers(BUFFER_COUNT, buffers);
    delete MG_State::pGLContext;
}
BENCHMARK(BM_CreateBufferObjectsAndBindBuffer)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_DeleteBufferObjects(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();
    std::vector<GLuint> buffers(BUFFER_COUNT);

    for (auto _ : state) {
        state.PauseTiming();
        GenBuffers(BUFFER_COUNT, buffers.data());
        for (GLuint i = 0; i < BUFFER_COUNT; i++) {
            GLenum target = (i % 2 == 0) ? GL_ARRAY_BUFFER : GL_UNIFORM_BUFFER;
            BindBuffer(target, buffers[i]);
        }
        state.ResumeTiming();

        DeleteBuffers(BUFFER_COUNT, buffers.data());
    }

    state.SetItemsProcessed(state.iterations() * BUFFER_COUNT);
    delete MG_State::pGLContext;
}
BENCHMARK(BM_DeleteBufferObjects)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_UpdateData(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    GLuint buffers[BUFFER_COUNT];
    GenBuffers(BUFFER_COUNT, buffers);

    for (GLuint i = 0; i < BUFFER_COUNT; i++)
        BindBuffer(GL_ARRAY_BUFFER, buffers[i]);

    std::vector<char> data(BUFFER_SIZE, 0);

    for (auto _ : state) {
        for (GLuint i = 0; i < BUFFER_COUNT; i++) {
            BindBuffer(GL_ARRAY_BUFFER, buffers[i]);
            BufferData(GL_ARRAY_BUFFER, BUFFER_SIZE, data.data(), GL_STATIC_DRAW);
        }
    }

    state.SetItemsProcessed(state.iterations() * BUFFER_COUNT);

    DeleteBuffers(BUFFER_COUNT, buffers);
    delete MG_State::pGLContext;
}
BENCHMARK(BM_UpdateData)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_MapBuffer(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    GLuint buffers[BUFFER_COUNT];
    GenBuffers(BUFFER_COUNT, buffers);
    for (GLuint i = 0; i < BUFFER_COUNT; i++)
        BindBuffer(GL_ARRAY_BUFFER, buffers[i]);

    std::vector<char> data(BUFFER_SIZE, 0);

    for (auto _ : state) {
        for (GLuint i = 0; i < BUFFER_COUNT; i++) {
            BindBuffer(GL_ARRAY_BUFFER, buffers[i]);
            BufferData(GL_ARRAY_BUFFER, BUFFER_SIZE, data.data(), GL_DYNAMIC_DRAW);

            void* ptr = MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            if (ptr) {
                memset(ptr, 1, BUFFER_SIZE);
                UnmapBuffer(GL_ARRAY_BUFFER);
            }
        }
    }

    state.SetItemsProcessed(state.iterations() * BUFFER_COUNT);

    DeleteBuffers(BUFFER_COUNT, buffers);
    delete MG_State::pGLContext;
}
BENCHMARK(BM_MapBuffer)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_CopyBufferSubData(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    GLuint buffers[BUFFER_COUNT];
    GenBuffers(BUFFER_COUNT, buffers);

    std::vector<char> data(BUFFER_SIZE, 0);

    for (GLuint i = 0; i < BUFFER_COUNT; i++) {
        GLenum target = (i % 2 == 0) ? GL_COPY_READ_BUFFER : GL_COPY_WRITE_BUFFER;
        BindBuffer(target, buffers[i]);
        BufferData(target, BUFFER_SIZE, data.data(), GL_STATIC_DRAW);
    }

    for (auto _ : state) {
        for (GLuint i = 0; i < BUFFER_COUNT; i += 2) {
            BindBuffer(GL_COPY_READ_BUFFER, buffers[i]);
            BindBuffer(GL_COPY_WRITE_BUFFER, buffers[i + 1]);
            CopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, BUFFER_SIZE);
        }
    }

    state.SetItemsProcessed(state.iterations() * (BUFFER_COUNT / 2));

    DeleteBuffers(BUFFER_COUNT, buffers);
    delete MG_State::pGLContext;
}
BENCHMARK(BM_CopyBufferSubData)->Unit(benchmark::kMillisecond)->UseRealTime();

static void BM_UpdateDataPartially(benchmark::State& state) {
    MG_State::pGLContext = new MG_State::GLState::GLContext();

    std::vector<GLuint> buffers(BUFFER_COUNT);
    std::vector<char> data(BUFFER_SIZE / 10, 1);

    GenBuffers(BUFFER_COUNT, buffers.data());
    for (GLuint i = 0; i < BUFFER_COUNT; i++) {
        BindBuffer(GL_ARRAY_BUFFER, buffers[i]);
        BufferData(GL_ARRAY_BUFFER, BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);
    }

    for (auto _ : state) {
        for (GLuint i = 0; i < BUFFER_COUNT; i++) {
            BindBuffer(GL_ARRAY_BUFFER, buffers[i]);
            BufferSubData(GL_ARRAY_BUFFER, BUFFER_SIZE / 2, data.size(), data.data());
        }
    }

    state.SetItemsProcessed(state.iterations() * BUFFER_COUNT);

    DeleteBuffers(BUFFER_COUNT, buffers.data());
    delete MG_State::pGLContext;
}
BENCHMARK(BM_UpdateDataPartially)->Unit(benchmark::kMillisecond)->UseRealTime();

BENCHMARK_MAIN();
