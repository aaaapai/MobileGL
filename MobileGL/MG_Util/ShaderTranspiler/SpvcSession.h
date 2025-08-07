//
// Created by Swung0x48 on 2025/8/7.
//

#ifndef SPVCSESSION_H
#define SPVCSESSION_H

#pragma once

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
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
        spvc_result Compile(const char** result) const;
        const char* GetLastErrorString() const;

    private:
        spvc_context context = nullptr;
        spvc_parsed_ir ir = nullptr;
        spvc_compiler compiler = nullptr;
        spvc_compiler_options compiler_options = nullptr;
        spvc_resources resources = nullptr;
    };
}
}
}


#endif //SPVCSESSION_H
