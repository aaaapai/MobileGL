// MobileGL - MobileGL/MG_Util/ShaderTranspiler/SpvcSession.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "SpvcSession.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            SpvcSession::SpvcSession(const Vector<unsigned int>& spirv) {
                const SpvId* p_spirv = spirv.data();
                size_t word_count = spirv.size();

                spvc_context_create(&context);
                spvc_context_parse_spirv(context, p_spirv, word_count, &ir);
                spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                             &compiler);
                spvc_compiler_create_shader_resources(compiler, &resources);
            }

            SpvcSession::SpvcSession(SpvcSession&& that) {
                std::swap(this->context, that.context);
                std::swap(this->compiler, that.compiler);
                std::swap(this->ir, that.ir);
                std::swap(this->compiler_options, that.compiler_options);
                std::swap(this->resources, that.resources);
            }

            SpvcSession& SpvcSession::operator=(SpvcSession&& that) {
                std::swap(this->context, that.context);
                std::swap(this->compiler, that.compiler);
                std::swap(this->ir, that.ir);
                std::swap(this->compiler_options, that.compiler_options);
                std::swap(this->resources, that.resources);
                return *this;
            }

            spvc_result SpvcSession::CreateOptions(spvc_compiler_options* options) {
                return spvc_compiler_create_compiler_options(compiler, options);
            }

            spvc_result SpvcSession::SetOptions(spvc_compiler_options options) {
                compiler_options = options;
                return spvc_compiler_install_compiler_options(compiler, options);
            }

            Vector<InterfaceVariable> SpvcSession::GetShaderInterface(spvc_resource_type resource_type) const {
                const spvc_reflected_resource* list = nullptr;
                size_t count = 0;
                spvc_resources_get_resource_list_for_type(resources, resource_type, &list, &count);

                Vector<InterfaceVariable> variables;
                for (size_t i = 0; i < count; ++i) {
                    if (spvc_compiler_has_decoration(compiler, list[i].id, SpvDecorationBuiltIn)) {
                        continue;
                    }

                    InterfaceVariable var;
                    var.name = list[i].name;
                    var.location = spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationLocation);
                    variables.push_back(var);
                }
                std::sort(variables.begin(), variables.end());
                return variables;
            }

            spvc_result SpvcSession::SetVertexAttribLocation(const UnorderedMap<String, Uint>& location) {
                // TODO: We should assert we're really dealing with vertex shader here

                SPVC_CHK_INIT
                const spvc_reflected_resource* list = nullptr;
                size_t count = 0;
                SPVC_CHK_RESULT(spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_STAGE_INPUT,
                                                                          &list, &count));
                for (size_t i = 0; i < count; ++i) {
                    auto& resource = list[i];
                    auto it = location.find(resource.name);
                    if (it != location.end()) {
                        // realize glBindVertexAttribLocation here
                        // it->second should be the location explicitly requested
                        spvc_compiler_set_decoration(compiler, resource.id, SpvDecorationLocation, it->second);
                    }
                }
                SPVC_CHK_RETURN
            }

            spvc_result SpvcSession::Compile(const char** result) {
                SPVC_CHK_INIT
                SPVC_CHK_RESULT(spvc_compiler_compile(compiler, result));
                // SPVC_CHK_RESULT(ParseMetaData());
                SPVC_CHK_RETURN
            }

            spvc_result SpvcSession::ParseMetaData() {
                SPVC_CHK_INIT

                metadata = SpvcMetadata();

                const spvc_reflected_resource* list = nullptr;
                size_t count = 0;

                SPVC_CHK_RESULT(spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER,
                                                                          &list, &count);)
                for (size_t i = 0; i < count; ++i) {
                    if (spvc_compiler_has_decoration(compiler, list[i].id, SpvDecorationBuiltIn)) {
                        continue;
                    }

                    if (strcmp(list[i].name, GLOBAL_UBO_NAME) == 0) {
                        spvc_type type = spvc_compiler_get_type_handle(compiler, list[i].base_type_id);
                        spvc_compiler_get_declared_struct_size(compiler, type, &metadata.uboSize);
                        size_t num_members = spvc_type_get_num_member_types(type);
                        for (size_t j = 0; j < num_members; ++j) {
                            const char* memberName = spvc_compiler_get_member_name(compiler, list[i].base_type_id, j);

                            unsigned memberOffset = 0;
                            SPVC_CHK_RESULT(spvc_compiler_type_struct_member_offset(compiler, type, j, &memberOffset);)
                            metadata.plainUniformOffsetsInUBO[memberName] = memberOffset;
                            SizeT memberSize = 0;
                            SPVC_CHK_RESULT(
                                spvc_compiler_get_declared_struct_member_size(compiler, type, j, &memberSize);)
                            metadata.plainUniformMemberSizesInBytes[memberName] = memberSize;

                            auto memberTypeId = spvc_type_get_member_type(type, j);
                            spvc_type memberType = spvc_compiler_get_type_handle(compiler, memberTypeId);
                            spvc_basetype basetype = spvc_type_get_basetype(memberType);
                            auto vectorSize = spvc_type_get_vector_size(memberType);
                            auto matCol = spvc_type_get_columns(memberType);
                            // auto dim = spvc_type_get_num_array_dimensions(type);
                            metadata.plainUniformMemberTypes[memberName] = {
                                .basetype = basetype,
                                .vectorSize = vectorSize,
                                .matCol = matCol,
                            };
                        }
                        SPVC_CHK_RETURN
                    }
                }
                // This means this spv binary does not have
                // auto-generated UBO in it
                return SPVC_ERROR_INVALID_SPIRV;
            }

            const SpvcMetadata& SpvcSession::GetMetadata() const {
                return metadata;
            }

            const char* SpvcSession::GetLastErrorString() const {
                return spvc_context_get_last_error_string(context);
            }

            SpvcSession::~SpvcSession() {
                spvc_context_destroy(context);
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
