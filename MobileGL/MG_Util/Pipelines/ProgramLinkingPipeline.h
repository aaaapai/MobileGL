//
// Created by Swung 0x48 on 2025/7/16.
//

#ifndef MG_UTIL_PIPELINES_PROGRAMLINKINGPIPELINE_H
#define MG_UTIL_PIPELINES_PROGRAMLINKINGPIPELINE_H

#include "../../Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace Pipeline {
    class ProgramLinkingPipeline: public IPipeline<ProgramLinkingPipeline, ProgramPayload> {
        explicit ProgramLinkingPipeline(std::function<void(ProgramPayload&)> callback);
        void Invoke(SharedPtr<ProgramPayload> payload);
    private:
        PipelineExecutor<ProgramPayload> executor_;
    };
}
}
}


#endif //MG_UTIL_PIPELINES_PROGRAMLINKINGPIPELINE_H
