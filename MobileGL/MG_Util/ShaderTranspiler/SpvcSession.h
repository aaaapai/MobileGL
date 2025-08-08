//
// Created by Swung0x48 on 2025/8/7.
//

#ifndef SPVCSESSION_H
#define SPVCSESSION_H

#pragma once

#define SPVC_CHK_INIT \
    auto __r = SPVC_SUCCESS;

#define SPVC_CHK_RESULT(res) \
    __r = res; \
    if (__r != SPVC_SUCCESS) { \
        return __r; \
    }

#define SPVC_CHK_RETURN \
    return __r;

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    struct SpvcMetadata {
        UnorderedMap<String, unsigned> plainUniformOffsetsInUBO;
    };

    class SpvcSession {
    public:
        SpvcSession() {}

        explicit SpvcSession(const Vector<unsigned int>& spirv);

        SpvcSession(SpvcSession&) = delete;

        SpvcSession(SpvcSession&& that);

        ~SpvcSession();

        SpvcSession& operator=(SpvcSession& session) = delete;

        SpvcSession& operator=(SpvcSession&& that);

        spvc_result CreateOptions(spvc_compiler_options *options);
        spvc_result SetOptions(spvc_compiler_options options);
        Vector<InterfaceVariable> GetShaderInterface(spvc_resource_type resource_type) const;
        spvc_result Compile(const char** result);
        const SpvcMetadata& GetMetadata() const;
        const char* GetLastErrorString() const;

    private:
        // Should be called once, and only once, for every SPIR-V binary
        spvc_result ParseMetaData();

        spvc_context context = nullptr;
        spvc_parsed_ir ir = nullptr;
        spvc_compiler compiler = nullptr;
        spvc_compiler_options compiler_options = nullptr;
        spvc_resources resources = nullptr;

        SpvcMetadata metadata;
    };
}
}
}


#endif //SPVCSESSION_H
