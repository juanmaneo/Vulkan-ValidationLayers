/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "state_tracker/image_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"

bool CoreChecks::ValidateDynamicStateIsSet(const CBDynamicFlags& state_status_cb, CBDynamicState dynamic_state,
                                           const LogObjectList& objlist, const Location& loc, const char* vuid) const {
    if (!state_status_cb[dynamic_state]) {
        return LogError(vuid, objlist, loc, "%s state is dynamic, but the command buffer never called %s.",
                        DynamicStateToString(dynamic_state), DescribeDynamicStateCommand(dynamic_state).c_str());
    }
    return false;
}

// Makes sure the vkCmdSet* call was called correctly prior to a draw
bool CoreChecks::ValidateGraphicsDynamicStateSetStatus(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const vvl::Pipeline& pipeline = *last_bound_state.pipeline_state;
    const vvl::DrawDispatchVuid& vuid = vvl::GetDrawDispatchVuid(loc.function);
    const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());

    // Verify vkCmdSet* calls since last bound pipeline
    const CBDynamicFlags unset_status_pipeline =
        (cb_state.dynamic_state_status.pipeline ^ pipeline.dynamic_state) & cb_state.dynamic_state_status.pipeline;
    if (unset_status_pipeline.any()) {
        skip |= LogError(vuid.dynamic_state_setting_commands_08608, objlist, loc,
                         "%s doesn't set up %s, but since the vkCmdBindPipeline, the related dynamic state commands (%s) have been "
                         "called in this command buffer.",
                         FormatHandle(pipeline).c_str(), DynamicStatesToString(unset_status_pipeline).c_str(),
                         DynamicStatesCommandsToString(unset_status_pipeline).c_str());
    }

    // build the mask of what has been set in the Pipeline, but yet to be set in the Command Buffer
    const CBDynamicFlags state_status_cb = ~((cb_state.dynamic_state_status.cb ^ pipeline.dynamic_state) & pipeline.dynamic_state);

    // VK_EXT_extended_dynamic_state
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_CULL_MODE, objlist, loc, vuid.dynamic_cull_mode_07840);
        skip |=
            ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_FRONT_FACE, objlist, loc, vuid.dynamic_front_face_07841);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, objlist, loc,
                                          vuid.dynamic_primitive_topology_07842);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE, objlist, loc,
                                          vuid.dynamic_depth_test_enable_07843);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, objlist, loc,
                                          vuid.dynamic_depth_write_enable_07844);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP, objlist, loc,
                                          vuid.dynamic_depth_compare_op_07845);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, objlist, loc,
                                          vuid.dynamic_depth_bound_test_enable_07846);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE, objlist, loc,
                                          vuid.dynamic_stencil_test_enable_07847);
        skip |=
            ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_OP, objlist, loc, vuid.dynamic_stencil_op_07848);
    }

    // VK_EXT_extended_dynamic_state2
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, objlist, loc,
                                          vuid.patch_control_points_04875);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, objlist, loc,
                                          vuid.rasterizer_discard_enable_04876);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, objlist, loc,
                                          vuid.depth_bias_enable_04877);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_EXT, objlist, loc, vuid.logic_op_04878);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, objlist, loc,
                                          vuid.primitive_restart_enable_04879);
    }

    // VK_EXT_extended_dynamic_state3
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_depth_clamp_enable_07620);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_POLYGON_MODE_EXT, objlist, loc,
                                          vuid.dynamic_polygon_mode_07621);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, objlist, loc,
                                          vuid.dynamic_rasterization_samples_07622);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT, objlist, loc,
                                          vuid.dynamic_sample_mask_07623);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT, objlist, loc,
                                          vuid.dynamic_tessellation_domain_origin_07619);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_alpha_to_coverage_enable_07624);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_alpha_to_one_enable_07625);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_logic_op_enable_07626);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT, objlist, loc,
                                          vuid.dynamic_rasterization_stream_07630);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT, objlist, loc,
                                          vuid.dynamic_conservative_rasterization_mode_07631);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT, objlist, loc,
                                          vuid.dynamic_extra_primitive_overestimation_size_07632);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_depth_clip_enable_07633);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_sample_locations_enable_07634);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT, objlist, loc,
                                          vuid.dynamic_provoking_vertex_mode_07636);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT, objlist, loc,
                                          vuid.dynamic_line_rasterization_mode_07637);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_line_stipple_enable_07638);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT, objlist, loc,
                                          vuid.dynamic_depth_clip_negative_one_to_one_07639);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_viewport_w_scaling_enable_07640);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV, objlist, loc,
                                          vuid.dynamic_viewport_swizzle_07641);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_coverage_to_color_enable_07642);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV, objlist, loc,
                                          vuid.dynamic_coverage_to_color_location_07643);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV, objlist, loc,
                                          vuid.dynamic_coverage_modulation_mode_07644);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_coverage_modulation_table_enable_07645);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV, objlist, loc,
                                          vuid.dynamic_coverage_modulation_table_07646);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_shading_rate_image_enable_07647);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_representative_fragment_test_enable_07648);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV, objlist, loc,
                                          vuid.dynamic_coverage_reduction_mode_07649);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, objlist, loc,
                                          vuid.dynamic_sample_locations_06666);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV, objlist, loc,
                                          vuid.dynamic_exclusive_scissor_enable_07878);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, objlist, loc,
                                          vuid.dynamic_exclusive_scissor_07879);
    }

    // VK_EXT_discard_rectangles
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_discard_rectangle_enable_07880);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT, objlist, loc,
                                          vuid.dynamic_discard_rectangle_mode_07881);
    }

    // VK_EXT_vertex_input_dynamic_state
    {
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
            pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT)) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE, objlist, loc,
                                              vuid.vertex_input_binding_stride_04913);
        }
        skip |=
            ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_EXT, objlist, loc, vuid.vertex_input_04914);
    }

    // VK_EXT_color_write_enable
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_color_write_enable_07749);
    }

    // VK_EXT_attachment_feedback_loop_dynamic_state
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, objlist, loc,
                                          vuid.dynamic_attachment_feedback_loop_08877);
    }

    if (const auto* rp_state = pipeline.RasterizationState()) {
        if (rp_state->depthBiasEnable == VK_TRUE) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS, objlist, loc,
                                              vuid.dynamic_depth_bias_07834);
        }

        // Any line topology
        const VkPrimitiveTopology topology = last_bound_state.GetPrimitiveTopology();
        if (IsValueIn(topology,
                      {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                       VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY})) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_WIDTH, objlist, loc,
                                              vuid.dynamic_line_width_07833);
            const auto *line_state = vku::FindStructInPNextChain<VkPipelineRasterizationLineStateCreateInfoKHR>(rp_state);
            if (line_state && line_state->stippledLineEnable) {
                skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE_KHR, objlist, loc,
                                                  vuid.dynamic_line_stipple_ext_07849);
            }
        }
    }

    if (pipeline.BlendConstantsEnabled()) {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_BLEND_CONSTANTS, objlist, loc,
                                          vuid.dynamic_blend_constants_07835);
    }

    if (pipeline.DepthStencilState()) {
        if (last_bound_state.IsDepthBoundTestEnable()) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS, objlist, loc,
                                              vuid.dynamic_depth_bounds_07836);
        }
        if (last_bound_state.IsStencilTestEnable()) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK, objlist, loc,
                                              vuid.dynamic_stencil_compare_mask_07837);
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK, objlist, loc,
                                              vuid.dynamic_stencil_write_mask_07838);
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_REFERENCE, objlist, loc,
                                              vuid.dynamic_stencil_reference_07839);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicState(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const auto pipeline_state = last_bound_state.pipeline_state;

    if (pipeline_state) {
        skip |= ValidateDrawDynamicStatePipeline(last_bound_state, loc);
    } else {
        skip |= ValidateDrawDynamicStateShaderObject(last_bound_state, loc);
    }

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const vvl::DrawDispatchVuid& vuid = vvl::GetDrawDispatchVuid(loc.function);
    if (!pipeline_state || pipeline_state->IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
            const auto* attachment = cb_state.active_attachments[i].image_view;
            if (attachment && attachment->create_info.format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) {
                const auto color_write_mask = cb_state.dynamic_state_value.color_write_masks[i];
                VkColorComponentFlags rgb = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
                if ((color_write_mask & rgb) != rgb && (color_write_mask & rgb) != 0) {
                    skip |= LogError(vuid.color_write_mask_09116, cb_state.Handle(), loc,
                                     "Render pass attachment %" PRIu32
                                     " has format VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, but the corresponding element of "
                                     "pColorWriteMasks is %s.",
                                     i, string_VkColorComponentFlags(color_write_mask).c_str());
                }
            }
        }
    }

    std::shared_ptr<const spirv::Module> vert_spirv_state;
    std::shared_ptr<const spirv::EntryPoint> vert_entrypoint;
    std::shared_ptr<const spirv::Module> frag_spirv_state;
    if (last_bound_state.pipeline_state) {
        for (const auto& stage_state : last_bound_state.pipeline_state->stage_states) {
            if (stage_state.GetStage() == VK_SHADER_STAGE_VERTEX_BIT) {
                vert_spirv_state = stage_state.spirv_state;
                vert_entrypoint = stage_state.entrypoint;
            }
            if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT) {
                frag_spirv_state = stage_state.spirv_state;
            }
        }
    } else {
        const auto& vertex_state = last_bound_state.GetShaderState(ShaderObjectStage::VERTEX);
        if (vertex_state) {
            vert_spirv_state = vertex_state->spirv;
            vert_entrypoint = vertex_state->entrypoint;
        }
        const auto& fragment_state = last_bound_state.GetShaderState(ShaderObjectStage::FRAGMENT);
        if (fragment_state) {
            frag_spirv_state = fragment_state->spirv;
        }
    }
    bool vertex_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::VERTEX);
    bool fragment_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::FRAGMENT);
    if (((pipeline_state && pipeline_state->IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) ||
         (!pipeline_state && vertex_shader_bound)) &&
        vert_entrypoint) {
        for (const auto* variable_ptr : vert_entrypoint->user_defined_interface_variables) {
            // Validate only input locations
            if (variable_ptr->storage_class != spv::StorageClass::StorageClassInput) {
                continue;
            }
            bool location_provided = false;
            for (uint32_t i = 0; i < cb_state.dynamic_state_value.vertex_attribute_descriptions.size(); ++i) {
                const auto& description = cb_state.dynamic_state_value.vertex_attribute_descriptions[i];
                if (variable_ptr->decorations.location == description.location) {
                    location_provided = true;

                    const uint32_t var_base_type_id = variable_ptr->base_type.ResultId();
                    const uint32_t attribute_type = spirv::GetFormatType(description.format);
                    const uint32_t var_numeric_type = vert_spirv_state->GetNumericType(var_base_type_id);

                    const bool attribute64 = vkuFormatIs64bit(description.format);
                    const bool shader64 = vert_spirv_state->GetBaseTypeInstruction(var_base_type_id)->GetBitWidth() == 64;

                    // first type check before doing 64-bit matching
                    if ((attribute_type & var_numeric_type) == 0) {
                        if (!enabled_features.legacyVertexAttributes || shader64) {
                            skip |=
                                LogError(vuid.vertex_input_08734, vert_spirv_state->handle(), loc,
                                         "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "].location (%" PRIu32
                                         ") with format %s but the vertex shader input is numeric type %s",
                                         i, description.location, string_VkFormat(description.format),
                                         vert_spirv_state->DescribeType(var_base_type_id).c_str());
                        }
                    } else if (attribute64 && !shader64) {
                        skip |= LogError(vuid.vertex_input_format_08936, vert_spirv_state->handle(), loc,
                                         "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "].location (%" PRIu32
                                         ") with a 64-bit format (%s) but the vertex shader input is 32-bit type (%s)",
                                         i, description.location, string_VkFormat(description.format),
                                         vert_spirv_state->DescribeType(var_base_type_id).c_str());
                    } else if (!attribute64 && shader64) {
                        skip |= LogError(vuid.vertex_input_format_08937, vert_spirv_state->handle(), loc,
                                         "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "].location (%" PRIu32
                                         ") with a 32-bit format (%s) but the vertex shader input is 64-bit type (%s)",
                                         i, description.location, string_VkFormat(description.format),
                                         vert_spirv_state->DescribeType(var_base_type_id).c_str());
                    } else if (attribute64 && shader64) {
                        const uint32_t attribute_components = vkuFormatComponentCount(description.format);
                        const uint32_t input_components = vert_spirv_state->GetNumComponentsInBaseType(&variable_ptr->base_type);
                        if (attribute_components < input_components) {
                            skip |=
                                LogError(vuid.vertex_input_format_09203, vert_spirv_state->handle(), loc,
                                         "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "].location (%" PRIu32
                                         ") with a %" PRIu32 "-wide 64-bit format (%s) but the vertex shader input is %" PRIu32
                                         "-wide. (64-bit vertex input don't have default values and require "
                                         "components to match what is used in the shader)",
                                         i, description.location, attribute_components, string_VkFormat(description.format),
                                         input_components);
                        }
                    }
                }
            }
            if (!location_provided) {
                skip |= LogError(vuid.vertex_input_format_07939, vert_spirv_state->handle(), loc,
                                 "Vertex shader uses input at location %" PRIu32
                                 ", but it was not provided with vkCmdSetVertexInputEXT().",
                                 variable_ptr->decorations.location);
            }
        }
    }

    // "a shader object bound to the VK_SHADER_STAGE_VERTEX_BIT stage or the bound graphics pipeline state was created with the
    // VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE"
    if ((pipeline_state && pipeline_state->IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) ||
        (!pipeline_state && vertex_shader_bound)) {
        if (!enabled_features.primitiveTopologyListRestart && cb_state.dynamic_state_value.primitive_restart_enable) {
            const VkPrimitiveTopology topology = last_bound_state.GetPrimitiveTopology();
            if (IsValueIn(topology, {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST})) {
                skip |= LogError(vuid.primitive_restart_list_09637, cb_state.Handle(), loc,
                                 "the topology set is %s, the primitiveTopologyListRestart feature was not enabled, but "
                                 "vkCmdSetPrimitiveRestartEnable last set primitiveRestartEnable to VK_TRUE.",
                                 string_VkPrimitiveTopology(topology));
            }
        }
    }

    if ((pipeline_state && pipeline_state->IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) || fragment_shader_bound) {
        if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT] &&
            cb_state.dynamic_state_value.sample_locations_enable) {
            if (cb_state.activeRenderPass->UsesDepthStencilAttachment(cb_state.GetActiveSubpass())) {
                for (uint32_t i = 0; i < cb_state.active_attachments.size(); i++) {
                    const auto* attachment = cb_state.active_attachments[i].image_view;
                    if (attachment && attachment->create_info.subresourceRange.aspectMask &
                                          (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                        if ((attachment->image_state->create_info.flags &
                             VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), frag_spirv_state->handle());
                            skip |=
                                LogError(vuid.sample_locations_enable_07484, objlist, loc,
                                         "Sample locations are enabled, but the depth/stencil attachment (%s) in the current "
                                         "subpass was not created with VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT.",
                                         FormatHandle(attachment->image_state->Handle()).c_str());
                        }
                        break;
                    }
                }
            }
            if ((!pipeline_state || pipeline_state->IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT)) &&
                cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT]) {
                VkSampleCountFlagBits rasterizationSamples =
                    (pipeline_state && !pipeline_state->IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT))
                        ? pipeline_state->MultisampleState()->rasterizationSamples
                        : cb_state.dynamic_state_value.rasterization_samples;
                VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
                DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, rasterizationSamples, &multisample_prop);
                const auto& gridSize = cb_state.dynamic_state_value.sample_locations_info.sampleLocationGridSize;
                if (SafeModulo(multisample_prop.maxSampleLocationGridSize.width, gridSize.width) != 0) {
                    const LogObjectList objlist(cb_state.Handle(), frag_spirv_state->handle());
                    skip |= LogError(vuid.sample_locations_enable_07485, objlist, loc,
                                     "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.width (%" PRIu32
                                     ") with rasterization samples %s is not evenly divided by "
                                     "sampleLocationsInfo.sampleLocationGridSize.width (%" PRIu32
                                     ") set with vkCmdSetSampleLocationsEXT().",
                                     multisample_prop.maxSampleLocationGridSize.width,
                                     string_VkSampleCountFlagBits(rasterizationSamples), gridSize.width);
                }
                if (SafeModulo(multisample_prop.maxSampleLocationGridSize.height, gridSize.height) != 0) {
                    const LogObjectList objlist(cb_state.Handle(), frag_spirv_state->handle());
                    skip |= LogError(vuid.sample_locations_enable_07486, objlist, loc,
                                     "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.height (%" PRIu32
                                     ") with rasterization samples %s is not evenly divided by "
                                     "sampleLocationsInfo.sampleLocationGridSize.height (%" PRIu32
                                     ") set with vkCmdSetSampleLocationsEXT().",
                                     multisample_prop.maxSampleLocationGridSize.height,
                                     string_VkSampleCountFlagBits(rasterizationSamples), gridSize.height);
                }
            }
            if (frag_spirv_state && frag_spirv_state->static_data_.uses_interpolate_at_sample) {
                const LogObjectList objlist(cb_state.Handle(), frag_spirv_state->handle());
                skip |= LogError(vuid.sample_locations_enable_07487, objlist, loc,
                                 "sampleLocationsEnable set with vkCmdSetSampleLocationsEnableEXT() was VK_TRUE, but fragment "
                                 "shader uses InterpolateAtSample instruction.");
            }
        }
    }

    if ((!pipeline_state || pipeline_state->IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT]) {
        const VkMultisampledRenderToSingleSampledInfoEXT* msrtss_info =
            cb_state.activeRenderPass->GetMSRTSSInfo(cb_state.GetActiveSubpass());
        if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable) {
            if (msrtss_info->rasterizationSamples != cb_state.dynamic_state_value.rasterization_samples) {
                LogObjectList objlist(cb_state.Handle(), frag_spirv_state->handle());
                skip |= LogError(vuid.rasterization_samples_09211, objlist, loc,
                                 "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampledEnable is VK_TRUE "
                                 "and VkMultisampledRenderToSingleSampledInfoEXT::rasterizationSamples are %s, but rasterization "
                                 "samples set with vkCmdSetRasterizationSamplesEXT() were %s.",
                                 string_VkSampleCountFlagBits(msrtss_info->rasterizationSamples),
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
        }
    }

    if (pipeline_state && cb_state.activeRenderPass->UsesDynamicRendering() &&
        (!IsExtEnabled(device_extensions.vk_ext_shader_object) || !last_bound_state.IsAnyGraphicsShaderBound())) {
        skip |= ValidateDrawRenderingAttachmentLocation(cb_state, *pipeline_state, loc, vuid);
        skip |= ValidateDrawRenderingInputAttachmentIndex(cb_state, *pipeline_state, loc, vuid);
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStatePipeline(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const vvl::Pipeline& pipeline = *last_bound_state.pipeline_state;
    skip |= ValidateGraphicsDynamicStateSetStatus(last_bound_state, loc);
    // Dynamic state was not set, will produce garbage when trying to read to values
    if (skip) return skip;

    const vvl::DrawDispatchVuid& vuid = vvl::GetDrawDispatchVuid(loc.function);

    // vkCmdSetDiscardRectangleEXT needs to be set on each rectangle
    const auto* discard_rectangle_state =
        vku::FindStructInPNextChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.GraphicsCreateInfo().pNext);
    if (discard_rectangle_state && pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
        for (uint32_t i = 0; i < discard_rectangle_state->discardRectangleCount; i++) {
            if (!cb_state.dynamic_state_value.discard_rectangles.test(i)) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.dynamic_discard_rectangle_07751, objlist, loc,
                    "vkCmdSetDiscardRectangleEXT was not set for discard rectangle index %" PRIu32 " for this command buffer.", i);
                break;
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT)) {
        if (!cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.color_blend_equation_07628, objlist, loc,
                             "Pipeline was created with VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT dynamic state, but "
                             "vkCmdSetColorBlendEquationEXT() was not called.");
        }
    }
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        if (!cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.color_write_mask_07629, objlist, loc,
                             "Pipeline was created with VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT dynamic state, but "
                             "vkCmdSetColorWriteMaskEXT() was not called.");
        }
    }
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT)) {
        if (!cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.color_blend_advanced_07635, objlist, loc,
                             "Pipeline was created with VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT dynamic state, but "
                             "vkCmdSetColorBlendAdvancedEXT() was not called.");
        }
    }

    // must set the state for all active color attachments in the current subpass
    for (const uint32_t &color_index : cb_state.active_color_attachments_index) {
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
            !cb_state.dynamic_state_value.color_blend_enable_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |=
                LogError(vuid.dynamic_color_blend_enable_07476, objlist, loc,
                         "vkCmdSetColorBlendEnableEXT was not set for color attachment index %" PRIu32 " for this command buffer.",
                         color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
            !cb_state.dynamic_state_value.color_blend_equation_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.dynamic_color_blend_equation_07477, objlist, loc,
                             "vkCmdSetColorBlendEquationEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT) &&
            !cb_state.dynamic_state_value.color_write_mask_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |=
                LogError(vuid.dynamic_color_write_mask_07478, objlist, loc,
                         "vkCmdSetColorWriteMaskEXT was not set for color attachment index %" PRIu32 " for this command buffer.",
                         color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) &&
            !cb_state.dynamic_state_value.color_blend_advanced_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.dynamic_color_blend_advanced_07479, objlist, loc,
                             "vkCmdSetColorBlendAdvancedEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             color_index);
        }
    }
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)) {
        if (!cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.color_blend_enable_07627, objlist, loc,
                             "Pipeline was created with VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT dynamic state, but "
                             "vkCmdSetColorBlendEnableEXT() was not called.");
        } else {
            const uint32_t attachment_count = static_cast<uint32_t>(cb_state.active_attachments.size());
            for (uint32_t i = 0; i < attachment_count; ++i) {
                if (!cb_state.dynamic_state_value.color_blend_enabled[i]) {
                    continue;
                }
                if (cb_state.dynamic_state_value.color_blend_advanced_attachments[i]) {
                    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) &&
                        attachment_count > phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                        skip |= LogError(vuid.blend_advanced_07480, objlist, loc,
                                         "Color Attachment %" PRIu32
                                         " blending is enabled, but the total color attachment count (%" PRIu32
                                         ") is greater than advancedBlendMaxColorAttachments (%" PRIu32 ").",
                                         i, attachment_count,
                                         phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments);
                        break;
                    }
                }

                const auto* attachment = cb_state.active_attachments[i].image_view;
                if (attachment && ((attachment->format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) == 0)) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                    skip |= LogError(
                        vuid.blend_feature_07470, objlist, loc,
                        "Color Attachment %" PRIu32
                        " has an image view format (%s) that doesn't support VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT.\n"
                        "(supported features: %s)",
                        i, string_VkFormat(attachment->create_info.format),
                        string_VkFormatFeatureFlags2(attachment->format_features).c_str());
                    break;
                }
            }
        }
    }
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT]) {
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel !=
                pipeline.MultisampleState()->rasterizationSamples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.sample_locations_07482, objlist, loc,
                    "sampleLocationsPerPixel set with vkCmdSetSampleLocationsEXT() was %s, but "
                    "VkPipelineMultisampleStateCreateInfo::rasterizationSamples from the pipeline was %s.",
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel),
                    string_VkSampleCountFlagBits(pipeline.MultisampleState()->rasterizationSamples));
            }
        } else {
            if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT] &&
                cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel !=
                    cb_state.dynamic_state_value.rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.sample_locations_07483, objlist, loc,
                    "sampleLocationsPerPixel set with vkCmdSetSampleLocationsEXT() was %s, but "
                    "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() was %s.",
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel),
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        if (enabled_features.variableMultisampleRate == VK_FALSE &&
            cb_state.activeRenderPass->UsesNoAttachment(cb_state.GetActiveSubpass())) {
            if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                    cb_state.GetActiveSubpassRasterizationSampleCount();
                subpass_rasterization_samples &&
                *subpass_rasterization_samples != cb_state.dynamic_state_value.rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.sample_locations_07471, objlist, loc,
                    "VkPhysicalDeviceFeatures::variableMultisampleRate is VK_FALSE and the rasterizationSamples set with "
                    "vkCmdSetRasterizationSamplesEXT() were %s but a previous draw used rasterization samples %" PRIu32 ".",
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                    *subpass_rasterization_samples);
            } else if ((cb_state.dynamic_state_value.rasterization_samples &
                        phys_dev_props.limits.framebufferNoAttachmentsSampleCounts) == 0) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.sample_locations_07471, objlist, loc,
                                 "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() are %s but this bit is not in "
                                 "framebufferNoAttachmentsSampleCounts (%s).",
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 string_VkSampleCountFlags(phys_dev_props.limits.framebufferNoAttachmentsSampleCounts).c_str());
            }
        }
    }

    if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT] && pipeline.MultisampleState()) {
        const auto sample_locations =
            vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(pipeline.MultisampleState()->pNext);
        if (sample_locations &&
            ((!pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT) && sample_locations->sampleLocationsEnable) ||
             (pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT) &&
              cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT] &&
              cb_state.dynamic_state_value.sample_locations_enable))) {
            VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
            DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, cb_state.dynamic_state_value.rasterization_samples,
                                                              &multisample_prop);

            if (SafeModulo(multisample_prop.maxSampleLocationGridSize.width,
                           sample_locations->sampleLocationsInfo.sampleLocationGridSize.width) != 0) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.sample_locations_enable_07936, objlist, loc,
                                 "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.width (%" PRIu32
                                 ") with rasterization samples %s is not evenly divided by "
                                 "VkMultisamplePropertiesEXT::sampleLocationGridSize.width (%" PRIu32 ").",
                                 multisample_prop.maxSampleLocationGridSize.width,
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 sample_locations->sampleLocationsInfo.sampleLocationGridSize.width);
            }
            if (SafeModulo(multisample_prop.maxSampleLocationGridSize.height,
                           sample_locations->sampleLocationsInfo.sampleLocationGridSize.height) != 0) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.sample_locations_enable_07937, objlist, loc,
                                 "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.height (%" PRIu32
                                 ") with rasterization samples %s is not evenly divided by "
                                 "VkMultisamplePropertiesEXT::sampleLocationGridSize.height (%" PRIu32 ").",
                                 multisample_prop.maxSampleLocationGridSize.height,
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 sample_locations->sampleLocationsInfo.sampleLocationGridSize.height);
            }
            if (sample_locations->sampleLocationsInfo.sampleLocationsPerPixel !=
                cb_state.dynamic_state_value.rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.sample_locations_enable_07938, objlist, loc,
                                 "Pipeline was created with "
                                 "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsInfo.sampleLocationsPerPixel %s "
                                 "which does not match rasterization samples (%s) set with vkCmdSetRasterizationSamplesEXT().",
                                 string_VkSampleCountFlagBits(sample_locations->sampleLocationsInfo.sampleLocationsPerPixel),
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT) &&
        !phys_dev_ext_props.conservative_rasterization_props.conservativePointAndLineRasterization &&
        (!pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) ||
         cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY])) {
        const VkPrimitiveTopology topology = last_bound_state.GetPrimitiveTopology();
        if (IsValueIn(topology,
                      {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                       VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY})) {
            if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT] &&
                cb_state.dynamic_state_value.conservative_rasterization_mode != VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.convervative_rasterization_07499, objlist, loc,
                    "Primitive topology is %s and conservativePointAndLineRasterization is VK_FALSE, but "
                    "conservativeRasterizationMode set with vkCmdSetConservativeRasterizationModeEXT() was %s",
                    string_VkPrimitiveTopology(topology),
                    string_VkConservativeRasterizationModeEXT(cb_state.dynamic_state_value.conservative_rasterization_mode));
            }
        }
    }

    // If Viewport or scissors are dynamic, verify that dynamic count matches PSO count.
    // Skip check if rasterization is disabled, if there is no viewport, or if viewport/scissors are being inherited.
    const bool dyn_viewport = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    const auto *rp_state = pipeline.RasterizationState();
    const auto *viewport_state = pipeline.ViewportState();
    if ((!rp_state || (rp_state->rasterizerDiscardEnable == VK_FALSE)) && viewport_state &&
        (cb_state.inheritedViewportDepths.empty())) {
        const bool dyn_scissor = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR);

        // NB (akeley98): Current validation layers do not detect the error where vkCmdSetViewport (or scissor) was called, but
        // the dynamic state set is overwritten by binding a graphics pipeline with static viewport (scissor) state.
        // This condition be detected by checking trashedViewportMask & viewportMask (trashedScissorMask & scissorMask) is
        // nonzero in the range of bits needed by the pipeline.
        if (dyn_viewport) {
            const auto required_viewports_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportMask & required_viewports_mask;
            if (missing_viewport_mask) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.dynamic_viewport_07831, objlist, loc,
                                 "Dynamic viewport(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                                 "to vkCmdSetViewport().",
                                 missing_viewport_mask);
            }
        }

        if (dyn_scissor) {
            const auto required_scissor_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorMask & required_scissor_mask;
            if (missing_scissor_mask) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.dynamic_scissor_07832, objlist, loc,
                                 "Dynamic scissor(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                                 "to vkCmdSetScissor().",
                                 missing_scissor_mask);
            }
        }

        const bool dyn_viewport_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const bool dyn_scissor_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);

        if (dyn_viewport_count && !dyn_scissor_count) {
            const auto required_viewport_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportWithCountMask & required_viewport_mask;
            if (missing_viewport_mask || !cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT]) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.viewport_count_03417, objlist, loc,
                                 "Dynamic viewport with count 0x%x are used by pipeline state object, but were not provided "
                                 "via calls to vkCmdSetViewportWithCountEXT().",
                                 missing_viewport_mask);
            }
        }

        if (dyn_scissor_count && !dyn_viewport_count) {
            const auto required_scissor_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorWithCountMask & required_scissor_mask;
            if (missing_scissor_mask || !cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT]) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.scissor_count_03418, objlist, loc,
                                 "Dynamic scissor with count 0x%x are used by pipeline state object, but were not provided via "
                                 "calls to vkCmdSetScissorWithCountEXT().",
                                 missing_scissor_mask);
            }
        }

        if (dyn_scissor_count && dyn_viewport_count) {
            if (cb_state.viewportWithCountMask != cb_state.scissorWithCountMask ||
                !cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT] ||
                !cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT]) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.viewport_scissor_count_03419, objlist, loc,
                                 "Dynamic viewport and scissor with count 0x%x are used by pipeline state object, but were not "
                                 "provided via matching calls to "
                                 "vkCmdSetViewportWithCountEXT and vkCmdSetScissorWithCountEXT().",
                                 (cb_state.viewportWithCountMask ^ cb_state.scissorWithCountMask));
            }
        }
    }

    // If inheriting viewports, verify that not using more than inherited.
    if (cb_state.inheritedViewportDepths.size() != 0 && dyn_viewport) {
        const uint32_t viewport_count = viewport_state->viewportCount;
        const uint32_t max_inherited = uint32_t(cb_state.inheritedViewportDepths.size());
        if (viewport_count > max_inherited) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.dynamic_state_inherited_07850, objlist, loc,
                             "Pipeline requires more viewports (%" PRIu32 ".) than inherited (viewportDepthCount = %" PRIu32 ".).",
                             viewport_count, max_inherited);
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT]) {
        const auto color_blend_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)->ColorBlendState();
        if (color_blend_state) {
            uint32_t blend_attachment_count = color_blend_state->attachmentCount;
            uint32_t dynamic_attachment_count = cb_state.dynamic_state_value.color_write_enable_attachment_count;
            if (dynamic_attachment_count < blend_attachment_count) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(
                    vuid.dynamic_color_write_enable_count_07750, objlist, loc,
                    "Currently bound pipeline was created with VkPipelineColorBlendStateCreateInfo::attachmentCount %" PRIu32
                    " and VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, but the number of attachments written by "
                    "vkCmdSetColorWriteEnableEXT() is %" PRIu32 ".",
                    blend_attachment_count, dynamic_attachment_count);
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)) {
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_MASK_EXT] &&
                cb_state.dynamic_state_value.samples_mask_samples < pipeline.MultisampleState()->rasterizationSamples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |=
                    LogError(vuid.sample_mask_07472, objlist, loc,
                             "Currently bound pipeline was created with VkPipelineMultisampleStateCreateInfo::rasterizationSamples "
                             "%s are greater than samples set with vkCmdSetSampleMaskEXT() were %s",
                             string_VkSampleCountFlagBits(pipeline.MultisampleState()->rasterizationSamples),
                             string_VkSampleCountFlagBits(cb_state.dynamic_state_value.samples_mask_samples));
            }
        } else {
            if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_SAMPLE_MASK_EXT] &&
                cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT]) {
                if (cb_state.dynamic_state_value.samples_mask_samples < cb_state.dynamic_state_value.rasterization_samples) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                    skip |= LogError(vuid.sample_mask_07473, objlist, loc,
                                     "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() %s are greater than samples "
                                     "set with vkCmdSetSampleMaskEXT() were %s",
                                     string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                     string_VkSampleCountFlagBits(cb_state.dynamic_state_value.samples_mask_samples));
                }
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT]) {
        if (!IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) &&
            !IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples)) {
            for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
                const AttachmentInfo& attachment_info = cb_state.active_attachments[i];
                const auto* attachment = attachment_info.image_view;
                if (attachment && !attachment_info.IsInput() && !attachment_info.IsResolve() &&
                    cb_state.dynamic_state_value.rasterization_samples != attachment->samples) {
                    skip |=
                        LogError(vuid.rasterization_sampled_07474, cb_state.Handle(), loc,
                                 "%s attachment samples %s does not match samples %s set with vkCmdSetRasterizationSamplesEXT().",
                                 attachment_info.Describe(cb_state.attachment_source, i).c_str(),
                                 string_VkSampleCountFlagBits(attachment->samples),
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
                }
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT) &&
        !enabled_features.primitivesGeneratedQueryWithNonZeroStreams &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT] &&
        cb_state.dynamic_state_value.rasterization_stream != 0) {
        bool pgq_active = false;
        for (const auto& active_query : cb_state.activeQueries) {
            auto query_pool_state = Get<vvl::QueryPool>(active_query.pool);
            if (query_pool_state && query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
                pgq_active = true;
                break;
            }
        }
        if (pgq_active) {
            skip |= LogError(
                vuid.primitives_generated_query_07481, cb_state.Handle(), loc,
                "Query with type VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT is active and primitivesGeneratedQueryWithNonZeroStreams "
                "feature is not enabled, but rasterizationStreams set with vkCmdSetRasterizationStreamEXT() was %" PRIu32,
                cb_state.dynamic_state_value.rasterization_stream);
        }
    }

    // VK_EXT_shader_tile_image
    {
        const bool dyn_depth_write_enable = pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
        const bool dyn_stencil_write_mask = pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        if ((dyn_depth_write_enable || dyn_stencil_write_mask) &&
            (pipeline.fragment_shader_state && pipeline.fragment_shader_state->fragment_entry_point)) {
            auto entrypoint = pipeline.fragment_shader_state->fragment_entry_point;
            const bool mode_early_fragment_test = entrypoint->execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit);
            const bool depth_read =
                pipeline.fragment_shader_state->fragment_shader->spirv->static_data_.has_shader_tile_image_depth_read;
            const bool stencil_read =
                pipeline.fragment_shader_state->fragment_shader->spirv->static_data_.has_shader_tile_image_stencil_read;

            if (depth_read && dyn_depth_write_enable && mode_early_fragment_test &&
                cb_state.dynamic_state_value.depth_write_enable) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.dynamic_depth_enable_08715, objlist, loc,
                                 "Fragment shader contains OpDepthAttachmentReadEXT, but depthWriteEnable parameter in the last "
                                 "call to vkCmdSetDepthWriteEnable is not false.");
            }

            if (stencil_read && dyn_stencil_write_mask && mode_early_fragment_test &&
                ((cb_state.dynamic_state_value.write_mask_front != 0) || (cb_state.dynamic_state_value.write_mask_back != 0))) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(vuid.dynamic_stencil_write_mask_08716, objlist, loc,
                                 "Fragment shader contains OpStencilAttachmentReadEXT, but writeMask parameter in the last "
                                 "call to vkCmdSetStencilWriteMask is not equal to 0 for both front (=%" PRIu32
                                 ") and back (=%" PRIu32 ").",
                                 cb_state.dynamic_state_value.write_mask_front, cb_state.dynamic_state_value.write_mask_back);
            }
        }
    }

    // Makes sure topology is compatible (in same topology class)
    // see vkspec.html#drawing-primitive-topology-class
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
        !phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted) {
        bool compatible_topology = false;
        const VkPrimitiveTopology pipeline_topology = pipeline.topology_at_rasterizer;
        const VkPrimitiveTopology dynamic_topology = cb_state.dynamic_state_value.primitive_topology;
        switch (pipeline_topology) {
            case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        if (!compatible_topology) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.primitive_topology_class_07500, objlist, loc,
                             "the last primitive topology %s state set by vkCmdSetPrimitiveTopology is "
                             "not compatible with the pipeline topology %s.",
                             string_VkPrimitiveTopology(dynamic_topology), string_VkPrimitiveTopology(pipeline_topology));
        }
    }

    if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) && cb_state.activeRenderPass->UsesDynamicRendering()) {
        const auto msrtss_info = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(
            cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.pNext);
        if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable &&
            msrtss_info->rasterizationSamples != pipeline.MultisampleState()->rasterizationSamples) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.rasterization_samples_07935, objlist, loc,
                             "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampledEnable is VK_TRUE, but "
                             "the rasterizationSamples (%" PRIu32 ") is not equal to rasterizationSamples (%" PRIu32
                             ") of the the currently bound pipeline.",
                             msrtss_info->rasterizationSamples, pipeline.MultisampleState()->rasterizationSamples);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawRenderingAttachmentLocation(const vvl::CommandBuffer& cb_state, const vvl::Pipeline& pipeline_state,
                                                         const Location& loc, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    if (!cb_state.rendering_attachments.set_color_locations) {
        return skip;
    }
    const uint32_t color_attachment_count = (uint32_t)cb_state.rendering_attachments.color_locations.size();

    // Default from spec
    uint32_t pipeline_color_count = 0;
    const uint32_t* pipeline_color_locations = nullptr;
    if (const auto* pipeline_location_info =
            vku::FindStructInPNextChain<VkRenderingAttachmentLocationInfoKHR>(pipeline_state.GraphicsCreateInfo().pNext)) {
        pipeline_color_count = pipeline_location_info->colorAttachmentCount;
        pipeline_color_locations = pipeline_location_info->pColorAttachmentLocations;
    } else if (const auto* pipeline_rendering_create_info = pipeline_state.GetPipelineRenderingCreateInfo()) {
        pipeline_color_count = pipeline_rendering_create_info->colorAttachmentCount;
    } else {
        return skip;  // hit dynamic rendering that is not using local read
    }

    if (pipeline_color_count != color_attachment_count) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip = LogError(vuid.dynamic_rendering_local_location_09548, objlist, loc,
                        "The pipeline VkRenderingAttachmentLocationInfoKHR::colorAttachmentCount is %" PRIu32
                        " but vkCmdSetRenderingAttachmentLocationsKHR last set colorAttachmentCount to %" PRIu32 "",
                        pipeline_color_count, color_attachment_count);
    } else if (pipeline_color_locations) {
        for (uint32_t i = 0; i < pipeline_color_count; i++) {
            if (pipeline_color_locations[i] != cb_state.rendering_attachments.color_locations[i]) {
                const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
                skip = LogError(
                    vuid.dynamic_rendering_local_location_09548, objlist, loc,
                    "The pipeline VkRenderingAttachmentLocationInfoKHR::pColorAttachmentLocations[%" PRIu32 "] is %" PRIu32
                    " but vkCmdSetRenderingAttachmentLocationsKHR last set pColorAttachmentLocations[%" PRIu32 "] to %" PRIu32 "",
                    i, pipeline_color_locations[i], i, cb_state.rendering_attachments.color_locations[i]);
                break;
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateDrawRenderingInputAttachmentIndex(const vvl::CommandBuffer& cb_state, const vvl::Pipeline& pipeline_state,
                                                           const Location& loc, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    if (!cb_state.rendering_attachments.set_color_indexes) {
        return skip;
    }

    const uint32_t color_index_count = (uint32_t)cb_state.rendering_attachments.color_indexes.size();

    // Default from spec
    uint32_t pipeline_color_count = 0;
    const uint32_t* pipeline_color_indexes = nullptr;
    const uint32_t* pipeline_depth_index = nullptr;
    const uint32_t* pipeline_stencil_index = nullptr;
    if (const auto* pipeline_index_info =
            vku::FindStructInPNextChain<VkRenderingInputAttachmentIndexInfoKHR>(pipeline_state.GraphicsCreateInfo().pNext)) {
        pipeline_color_count = pipeline_index_info->colorAttachmentCount;
        pipeline_color_indexes = pipeline_index_info->pColorAttachmentInputIndices;
        pipeline_depth_index = pipeline_index_info->pDepthInputAttachmentIndex;
        pipeline_stencil_index = pipeline_index_info->pStencilInputAttachmentIndex;
    } else if (const auto* pipeline_rendering_create_info = pipeline_state.GetPipelineRenderingCreateInfo()) {
        pipeline_color_count = pipeline_rendering_create_info->colorAttachmentCount;
    } else {
        return skip;  // hit dynamic rendering that is not using local read
    }

    if (pipeline_color_count != color_index_count) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip = LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                        "The pipeline VkRenderingInputAttachmentIndexInfoKHR::colorAttachmentCount is %" PRIu32
                        " but vkCmdSetRenderingInputAttachmentIndicesKHR last set colorAttachmentCount to %" PRIu32 "",
                        pipeline_color_count, color_index_count);
    } else if (pipeline_color_indexes) {
        for (uint32_t i = 0; i < pipeline_color_count; i++) {
            if (pipeline_color_indexes[i] != cb_state.rendering_attachments.color_indexes[i]) {
                const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
                skip = LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                                "The pipeline VkRenderingInputAttachmentIndexInfoKHR::pColorAttachmentInputIndices[%" PRIu32
                                "] is %" PRIu32
                                " but vkCmdSetRenderingInputAttachmentIndicesKHR last set pColorAttachmentInputIndices[%" PRIu32
                                "] to %" PRIu32 "",
                                i, pipeline_color_indexes[i], i, cb_state.rendering_attachments.color_indexes[i]);
                break;
            }
        }
    }

    if ((!pipeline_depth_index || !cb_state.rendering_attachments.depth_index) &&
        (pipeline_depth_index != cb_state.rendering_attachments.depth_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip = LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                        "The pipeline VkRenderingInputAttachmentIndexInfoKHR::pDepthInputAttachmentIndex is 0x%p but "
                        "vkCmdSetRenderingInputAttachmentIndicesKHR last set pDepthInputAttachmentIndex to 0x%p",
                        pipeline_depth_index, cb_state.rendering_attachments.depth_index);
    } else if (pipeline_depth_index && cb_state.rendering_attachments.depth_index &&
               (*pipeline_depth_index != *cb_state.rendering_attachments.depth_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip = LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                        "The pipeline VkRenderingInputAttachmentIndexInfoKHR::pDepthInputAttachmentIndex value is %" PRIu32
                        " but vkCmdSetRenderingInputAttachmentIndicesKHR last set pDepthInputAttachmentIndex value to %" PRIu32 "",
                        *pipeline_depth_index, *cb_state.rendering_attachments.depth_index);
    }

    if ((!pipeline_stencil_index || !cb_state.rendering_attachments.stencil_index) &&
        (pipeline_stencil_index != cb_state.rendering_attachments.stencil_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip = LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                        "The pipeline VkRenderingInputAttachmentIndexInfoKHR::pStencilInputAttachmentIndex is 0x%p but "
                        "vkCmdSetRenderingInputAttachmentIndicesKHR last set pStencilInputAttachmentIndex to 0x%p",
                        pipeline_stencil_index, cb_state.rendering_attachments.stencil_index);
    } else if (pipeline_stencil_index && cb_state.rendering_attachments.stencil_index &&
               (*pipeline_stencil_index != *cb_state.rendering_attachments.stencil_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        skip =
            LogError(vuid.dynamic_rendering_local_index_09549, objlist, loc,
                     "The pipeline VkRenderingInputAttachmentIndexInfoKHR::pStencilInputAttachmentIndex value is %" PRIu32
                     " but vkCmdSetRenderingInputAttachmentIndicesKHR last set pStencilInputAttachmentIndex value to %" PRIu32 "",
                     *pipeline_stencil_index, *cb_state.rendering_attachments.stencil_index);
    }
    return skip;
}

bool CoreChecks::ValidateDrawDynamicStateShaderObject(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const vvl::DrawDispatchVuid& vuid = vvl::GetDrawDispatchVuid(loc.function);
    const LogObjectList objlist(cb_state.Handle());

    bool graphics_shader_bound = false;
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::VERTEX);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::TESSELLATION_CONTROL);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::TESSELLATION_EVALUATION);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::GEOMETRY);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::FRAGMENT);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::TASK);
    graphics_shader_bound |= last_bound_state.IsValidShaderBound(ShaderObjectStage::MESH);
    bool vertex_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::VERTEX);
    bool tessev_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::TESSELLATION_EVALUATION);
    bool geom_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::GEOMETRY);
    bool fragment_shader_bound = last_bound_state.IsValidShaderBound(ShaderObjectStage::FRAGMENT);

    if (!graphics_shader_bound) {
        return skip;
    }

    const auto isLineTopology = [](VkPrimitiveTopology topology) {
        return IsValueIn(topology,
                         {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                          VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY});
    };

    bool tess_shader_line_topology =
        tessev_shader_bound &&
        isLineTopology(last_bound_state.GetShaderState(ShaderObjectStage::TESSELLATION_EVALUATION)->GetTopology());
    bool geom_shader_line_topology =
        geom_shader_bound && isLineTopology(last_bound_state.GetShaderState(ShaderObjectStage::GEOMETRY)->GetTopology());

    if (!cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ||
        !cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) {
        skip |= LogError(vuid.viewport_and_scissor_with_count_08635, cb_state.Handle(), loc,
                         "Graphics shader objects are bound, but vkCmdSetViewportWithCount() and "
                         "vkCmdSetScissorWithCount() were not both called.");
    } else if (cb_state.dynamic_state_value.viewport_count != cb_state.dynamic_state_value.scissor_count) {
        skip |= LogError(vuid.viewport_and_scissor_with_count_08635, cb_state.Handle(), loc,
                         "Graphics shader objects are bound, but viewportCount set with vkCmdSetViewportWithCount() was %" PRIu32
                         " and scissorCount set with vkCmdSetScissorWithCount() was %" PRIu32 ".",
                         cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.scissor_count);
    }
    if (IsExtEnabled(device_extensions.vk_nv_clip_space_w_scaling) &&
        cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV) &&
        cb_state.dynamic_state_value.viewport_w_scaling_enable &&
        cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV) &&
        cb_state.dynamic_state_value.viewport_w_scaling_count < cb_state.dynamic_state_value.viewport_count) {
        skip |= LogError(vuid.viewport_w_scaling_08636, cb_state.Handle(), loc,
                         "Graphics shader objects are bound, but viewportCount set with vkCmdSetViewportWithCount() was %" PRIu32
                         " and viewportCount set with vkCmdSetViewportWScalingNV() was %" PRIu32 ".",
                         cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.viewport_w_scaling_count);
    }
    if (enabled_features.exclusiveScissor) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV, objlist,
                                          loc, vuid.set_exclusive_scissor_enable_09235);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV) &&
            !cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV)) {
            bool exclusiveScissorEnabled = false;
            for (uint32_t i = 0; i < cb_state.dynamic_state_value.exclusive_scissor_enable_count; ++i) {
                if (cb_state.dynamic_state_value
                        .exclusive_scissor_enables[cb_state.dynamic_state_value.exclusive_scissor_enable_first + i]) {
                    exclusiveScissorEnabled = true;
                    break;
                }
            }
            if (exclusiveScissorEnabled) {
                skip |=
                    LogError(vuid.exclusive_scissor_08638, cb_state.Handle(), loc,
                             "Graphics shader objects are bound, an element of pExclusiveScissorEnables set with "
                             "vkCmdSetExclusiveScissorEnableNV() was VK_TRUE, but vkCmdSetExclusiveScissorNV() was not called.");
            }
        }
    }

    skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, objlist, loc,
                                      vuid.set_rasterizer_discard_enable_08639);
    if (!cb_state.dynamic_state_value.rasterizer_discard_enable) {
        for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
            const auto* attachment = cb_state.active_attachments[i].image_view;
            if (attachment && vkuFormatIsColor(attachment->create_info.format) &&
                (attachment->format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) == 0 &&
                cb_state.dynamic_state_value.color_blend_enabled[i] == VK_TRUE) {
                skip |= LogError(vuid.set_color_blend_enable_08643, cb_state.Handle(), loc,
                                 "Render pass attachment %" PRIu32
                                 " has format %s, which does not have VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, but "
                                 "pColorBlendEnables[%" PRIu32 "] set with vkCmdSetColorBlendEnableEXT() was VK_TRUE.",
                                 i, string_VkFormat(attachment->create_info.format), i);
            }
        }
        if (!IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) &&
            !IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) &&
            enabled_features.multisampledRenderToSingleSampled == VK_FALSE &&
            cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
                const auto* attachment = cb_state.active_attachments[i].image_view;
                if (attachment && cb_state.dynamic_state_value.rasterization_samples != attachment->samples) {
                    skip |= LogError(vuid.set_rasterization_samples_08644, cb_state.Handle(), loc,
                                     "Render pass attachment %" PRIu32
                                     " samples %s does not match samples %s set with vkCmdSetRasterizationSamplesEXT().",
                                     i, string_VkSampleCountFlagBits(attachment->samples),
                                     string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
                }
            }
        }
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_POLYGON_MODE_EXT, objlist, loc,
                                          vuid.set_polygon_mode_08651);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, objlist,
                                          loc, vuid.set_rasterization_samples_08652);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT, objlist, loc,
                                          vuid.set_sample_mask_08653);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT, objlist,
                                          loc, vuid.set_alpha_to_coverage_enable_08654);
        if (enabled_features.alphaToOne) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT, objlist,
                                              loc, vuid.set_alpha_to_one_enable_08655);
        }
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_CULL_MODE, objlist, loc,
                                          vuid.set_cull_mode_08627);

        if ((cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_CULL_MODE) &&
             cb_state.dynamic_state_value.cull_mode != VK_CULL_MODE_NONE) ||
            cb_state.dynamic_state_value.stencil_test_enable == VK_TRUE) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_FRONT_FACE, objlist, loc,
                                              vuid.set_front_face_08628);
        }
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE, objlist, loc,
                                          vuid.set_depth_test_enable_08629);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, objlist, loc,
                                          vuid.set_depth_write_enable_08630);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE) && cb_state.dynamic_state_value.depth_test_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP, objlist, loc,
                                              vuid.set_depth_comapre_op_08631);
        }
        if (enabled_features.depthBounds) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, objlist,
                                              loc, vuid.set_depth_bounds_test_enable_08632);
        }
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE) &&
            cb_state.dynamic_state_value.depth_bounds_test_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS, objlist, loc,
                                              vuid.set_depth_bounds_08622);
        }
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, objlist, loc,
                                          vuid.set_depth_bias_enable_08640);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE) && cb_state.dynamic_state_value.depth_bias_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_BIAS, objlist, loc,
                                              vuid.set_depth_bias_08620);
        }
        if (enabled_features.depthClamp) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, objlist,
                                              loc, vuid.set_depth_clamp_enable_08650);
        }

        if (IsExtEnabled(device_extensions.vk_ext_conservative_rasterization)) {
            skip |=
                ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
                                          objlist, loc, vuid.set_conservative_rasterization_mode_08661);
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT) &&
                cb_state.dynamic_state_value.conservative_rasterization_mode ==
                    VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb,
                                                  CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT, objlist, loc,
                                                  vuid.set_extra_primitive_overestimation_size_08662);
            }
        }
        if (IsExtEnabled(device_extensions.vk_ext_sample_locations)) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
                                              objlist, loc, vuid.set_sample_locations_enable_08664);
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT) &&
                cb_state.dynamic_state_value.sample_locations_enable) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, objlist,
                                                  loc, vuid.set_sample_locations_08626);
            }
        }
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE, objlist, loc,
                                          vuid.set_stencil_test_enable_08633);
        if (cb_state.dynamic_state_value.stencil_test_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_STENCIL_OP, objlist, loc,
                                              vuid.set_stencil_op_08634);
        }
        if (IsExtEnabled(device_extensions.vk_ext_provoking_vertex) && vertex_shader_bound) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT, objlist,
                                              loc, vuid.set_provoking_vertex_mode_08665);
        }
        if (IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples)) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV,
                                              objlist, loc, vuid.set_coverage_modulation_mode_08678);
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV) &&
                cb_state.dynamic_state_value.coverage_modulation_mode != VK_COVERAGE_MODULATION_MODE_NONE_NV) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb,
                                                  CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV, objlist, loc,
                                                  vuid.set_coverage_modulation_table_enable_08679);
            }
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV) &&
                cb_state.dynamic_state_value.coverage_modulation_table_enable) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
                                                  objlist, loc, vuid.set_coverage_modulation_table_08680);
            }
        }
        if (enabled_features.coverageReductionMode) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV,
                                              objlist, loc, vuid.set_coverage_reduction_mode_08683);
        }
        if (enabled_features.representativeFragmentTest) {
            skip |=
                ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV,
                                          objlist, loc, vuid.set_representative_fragment_test_enable_08682);
        }
        if (enabled_features.shadingRateImage) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV,
                                              objlist, loc, vuid.set_viewport_coarse_sample_order_09233);
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV,
                                              objlist, loc, vuid.set_shading_rate_image_enable_08681);
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV) &&
                cb_state.dynamic_state_value.shading_rate_image_enable) {
                skip |=
                    ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV,
                                              objlist, loc, vuid.set_viewport_shading_rate_palette_09234);
                if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV) &&
                    cb_state.dynamic_state_value.shading_rate_palette_count < cb_state.dynamic_state_value.viewport_count) {
                    skip |= LogError(
                        vuid.shading_rate_palette_08637, cb_state.Handle(), loc,
                        "Graphics shader objects are bound, but viewportCount set with vkCmdSetViewportWithCount() was %" PRIu32
                        " and viewportCount set with vkCmdSetViewportShadingRatePaletteNV() was %" PRIu32 ".",
                        cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.shading_rate_palette_count);
                }
            }
        }
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE) && cb_state.dynamic_state_value.stencil_test_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK, objlist, loc,
                                              vuid.set_stencil_compare_mask_08623);
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK, objlist, loc,
                                              vuid.set_stencil_write_mask_08624);
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_STENCIL_REFERENCE, objlist, loc,
                                              vuid.set_stencil_reference_08625);
        }
        const bool line_rasterization_extension =
            IsExtEnabled(device_extensions.vk_ext_line_rasterization) || IsExtEnabled(device_extensions.vk_khr_line_rasterization);
        if (line_rasterization_extension && !cb_state.dynamic_state_value.rasterizer_discard_enable) {
            if (cb_state.dynamic_state_value.polygon_mode == VK_POLYGON_MODE_LINE) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
                                                  objlist, loc, vuid.set_line_rasterization_mode_08666);
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                                  objlist, loc, vuid.set_line_stipple_enable_08669);
            }
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT) &&
                cb_state.dynamic_state_value.stippled_line_enable) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_STIPPLE_KHR, objlist, loc,
                                                  vuid.set_line_stipple_08672);
            }
        }
        if (vertex_shader_bound) {
            if (isLineTopology(cb_state.dynamic_state_value.primitive_topology)) {
                if (line_rasterization_extension) {
                    skip |=
                        ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
                                                  objlist, loc, vuid.set_line_rasterization_mode_08667);
                    skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                                      objlist, loc, vuid.set_line_stipple_enable_08670);
                }
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_WIDTH, objlist, loc,
                                                  vuid.set_line_width_08618);
            }
        }

        if ((tessev_shader_bound && tess_shader_line_topology) || (geom_shader_bound && geom_shader_line_topology)) {
            if (line_rasterization_extension) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
                                                  objlist, loc, vuid.set_line_rasterization_mode_08668);
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                                  objlist, loc, vuid.set_line_stipple_enable_08671);
            }
        }
    }
    if (enabled_features.depthClipEnable) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT, objlist, loc,
                                          vuid.set_depth_clip_enable_08663);
    }
    if (enabled_features.depthClipControl) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,
                                          objlist, loc, vuid.set_depth_clip_negative_one_to_one_08673);
    }
    if (IsExtEnabled(device_extensions.vk_nv_clip_space_w_scaling)) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV, objlist,
                                          loc, vuid.set_viewport_w_scaling_enable_08674);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV) &&
            cb_state.dynamic_state_value.viewport_w_scaling_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, objlist,
                                              loc, vuid.set_clip_space_w_scaling_09232);
        }
    }
    if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_POLYGON_MODE_EXT) &&
        cb_state.dynamic_state_value.polygon_mode == VK_POLYGON_MODE_LINE) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_WIDTH, objlist, loc,
                                          vuid.set_line_width_08617);
    }

    if (vertex_shader_bound) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, objlist, loc,
                                          vuid.dynamic_primitive_topology_07842);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, objlist, loc,
                                          vuid.primitive_restart_enable_04879);
    }
    if (tessev_shader_bound) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, objlist, loc,
                                          vuid.patch_control_points_04875);
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
                                          objlist, loc, vuid.set_tessellation_domain_origin_09237);
    }
    if ((tessev_shader_bound && tess_shader_line_topology) || (geom_shader_bound && geom_shader_line_topology)) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LINE_WIDTH, objlist, loc,
                                          vuid.set_line_width_08619);
    }
    if (geom_shader_bound) {
        if (enabled_features.geometryStreams) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT, objlist,
                                              loc, vuid.set_rasterization_streams_08660);
        }
    }
    if (fragment_shader_bound) {
        if (!cb_state.dynamic_state_value.rasterizer_discard_enable) {
            if (enabled_features.logicOp) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT, objlist,
                                                  loc, vuid.set_logic_op_enable_08656);
            }
            if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT) && cb_state.dynamic_state_value.logic_op_enable) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_LOGIC_OP_EXT, objlist, loc,
                                                  vuid.set_logic_op_08641);
            }

            const uint32_t attachment_count = cb_state.activeRenderPass->GetDynamicRenderingColorAttachmentCount();
            if (attachment_count > 0) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                                                  objlist, loc, vuid.set_color_blend_enable_08657);
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                                  objlist, loc, vuid.set_blend_equation_09418);
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT, objlist,
                                                  loc, vuid.set_color_write_mask_08659);
            }

            const std::array<VkBlendFactor, 4> const_factors = {
                VK_BLEND_FACTOR_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, VK_BLEND_FACTOR_CONSTANT_ALPHA,
                VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA};
            for (uint32_t i = 0; i < attachment_count; ++i) {
                if (!cb_state.dynamic_state_value.color_blend_enable_attachments[i]) {
                    skip |= LogError(vuid.set_blend_advanced_09417, objlist, loc,
                                     "%s state not set for this command buffer for attachment %" PRIu32 ".",
                                     DynamicStateToString(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT), i);
                } else if (cb_state.dynamic_state_value.color_blend_enabled[i]) {
                    if (!cb_state.dynamic_state_value.color_blend_equation_attachments[i]) {
                        skip |= LogError(vuid.set_color_blend_equation_08658, objlist, loc,
                                         "%s state not set for this command buffer for attachment %" PRIu32 ".",
                                         DynamicStateToString(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT), i);
                    } else if (cb_state.dynamic_state_value.color_blend_equation_attachments[i]) {
                        const auto& eq = cb_state.dynamic_state_value.color_blend_equations[i];
                        if (std::find(const_factors.begin(), const_factors.end(), eq.srcColorBlendFactor) != const_factors.end() ||
                            std::find(const_factors.begin(), const_factors.end(), eq.dstColorBlendFactor) != const_factors.end() ||
                            std::find(const_factors.begin(), const_factors.end(), eq.srcAlphaBlendFactor) != const_factors.end() ||
                            std::find(const_factors.begin(), const_factors.end(), eq.dstAlphaBlendFactor) != const_factors.end()) {
                            if (!cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_BLEND_CONSTANTS)) {
                                skip |= LogError(vuid.set_blend_constants_08621, objlist, loc,
                                                 "%s state not set for this command buffer for attachment %" PRIu32 ".",
                                                 DynamicStateToString(CB_DYNAMIC_STATE_BLEND_CONSTANTS), i);
                            }
                        }
                    }
                }
                if (!cb_state.dynamic_state_value.color_write_mask_attachments[i]) {
                    skip |= LogError(vuid.set_color_write_09419, objlist, loc,
                                     "%s state not set for this command buffer for attachment %" PRIu32 ".",
                                     DynamicStateToString(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT), i);
                }
            }
            if (IsExtEnabled(device_extensions.vk_ext_blend_operation_advanced)) {
                if (!cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
                    !cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT)) {
                    skip |= LogError(vuid.set_blend_operation_advance_09416, objlist, loc,
                                     "Neither %s nor %s state were set for this command buffer.",
                                     DynamicStateToString(CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT),
                                     DynamicStateToString(CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT));
                }
            }
            if (enabled_features.pipelineFragmentShadingRate) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR,
                                                  objlist, loc, vuid.set_fragment_shading_rate_09238);
            }
            if (enabled_features.attachmentFeedbackLoopDynamicState) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb,
                                                  CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, objlist, loc,
                                                  vuid.set_attachment_feedback_loop_enable_08880);
            }
            if (IsExtEnabled(device_extensions.vk_nv_fragment_coverage_to_color)) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV,
                                                  objlist, loc, vuid.set_coverage_to_color_enable_08676);
                if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV) &&
                    cb_state.dynamic_state_value.coverage_to_color_enable) {
                    skip |=
                        ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
                                                  objlist, loc, vuid.set_coverage_to_color_location_08677);
                    if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV)) {
                        VkFormat format = VK_FORMAT_UNDEFINED;
                        if (cb_state.dynamic_state_value.coverage_to_color_location < cb_state.active_attachments.size()) {
                            format = cb_state.active_attachments[cb_state.dynamic_state_value.coverage_to_color_location]
                                         .image_view->create_info.format;
                        }
                        if (!IsValueIn(format, {VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT, VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT,
                                                VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT})) {
                            skip |= LogError(vuid.set_coverage_to_color_location_09420, cb_state.Handle(), loc,
                                             "Color attachment format selected by coverageToColorLocation (%" PRIu32 ") is %s.",
                                             cb_state.dynamic_state_value.coverage_to_color_location, string_VkFormat(format));
                        }
                    }
                }
            }
            if (enabled_features.colorWriteEnable) {
                if (!cb_state.dynamic_state_value.rasterizer_discard_enable) {
                    if (!cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
                        skip |= LogError(vuid.set_color_write_enable_08646, cb_state.Handle(), loc,
                                         "Fragment shader object is bound and rasterization is enabled, but "
                                         "vkCmdSetColorWriteEnableEXT() was not called.");
                    }
                }
                if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT) &&
                    cb_state.dynamic_state_value.color_write_enable_attachment_count < cb_state.GetDynamicColorAttachmentCount()) {
                    skip |= LogError(vuid.set_color_write_enable_08647, cb_state.Handle(), loc,
                                     "vkCmdSetColorWriteEnableEXT() was called with attachmentCount %" PRIu32
                                     ", but current render pass attachmnet count is %" PRIu32 ".",
                                     cb_state.dynamic_state_value.color_write_enable_attachment_count,
                                     cb_state.GetDynamicColorAttachmentCount());
                }
            }
        }
    }

    if (IsExtEnabled(device_extensions.vk_nv_viewport_swizzle)) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV, objlist, loc,
                                          vuid.set_viewport_swizzle_08675);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV)) {
            if (cb_state.dynamic_state_value.viewport_swizzle_count < cb_state.dynamic_state_value.viewport_count) {
                skip |=
                    LogError(vuid.set_viewport_swizzle_09421, cb_state.Handle(), loc,
                             "viewportCount (%" PRIu32 ") set with vkCmdSetViewportSwizzleNV() is less than viewportCount (%" PRIu32
                             ") set with vkCmdSetViewportWithCount()",
                             cb_state.dynamic_state_value.viewport_swizzle_count, cb_state.dynamic_state_value.viewport_count);
            }
        }
    }
    if (IsExtEnabled(device_extensions.vk_ext_discard_rectangles)) {
        skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT, objlist,
                                          loc, vuid.set_discard_rectangles_enable_08648);
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT) &&
            cb_state.dynamic_state_value.discard_rectangle_enable) {
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT,
                                              objlist, loc, vuid.set_discard_rectangles_mode_08649);
            skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT, objlist,
                                              loc, vuid.set_discard_rectangle_09236);
        }
    }
    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports) {
        for (uint32_t stage = 0; stage < kShaderObjectStageCount; ++stage) {
            const auto shader_stage = last_bound_state.GetShaderState(static_cast<ShaderObjectStage>(stage));
            if (shader_stage && shader_stage->entrypoint && shader_stage->entrypoint->written_builtin_primitive_shading_rate_khr) {
                skip |= ValidateDynamicStateIsSet(cb_state.dynamic_state_status.cb, CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, objlist,
                                                  loc, vuid.set_viewport_with_count_08642);
                if (cb_state.dynamic_state_value.viewport_count != 1) {
                    skip |= LogError(
                        vuid.set_viewport_with_count_08642, cb_state.Handle(), loc,
                        "primitiveFragmentShadingRateWithMultipleViewports is not supported and shader stage %s uses "
                        "PrimitiveShadingRateKHR, but viewportCount set with vkCmdSetViewportWithCount was %" PRIu32 ".",
                        string_VkShaderStageFlagBits(shader_stage->create_info.stage), cb_state.dynamic_state_value.viewport_count);
                }
                break;
            }
        }
    }

    if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT) &&
        cb_state.dynamic_state_value.alpha_to_coverage_enable) {
        const auto fragment_shader_stage = last_bound_state.GetShaderState(ShaderObjectStage::FRAGMENT);
        if (fragment_shader_stage && fragment_shader_stage->entrypoint &&
            !fragment_shader_stage->entrypoint->has_alpha_to_coverage_variable) {
            const LogObjectList frag_objlist(cb_state.Handle(), fragment_shader_stage->Handle());
            skip |= LogError(vuid.alpha_component_word_08920, frag_objlist, loc,
                             "alphaToCoverageEnable is set, but fragment shader doesn't declare a variable that covers "
                             "Location 0, Component 0.");
        }
    }

    // Resolve mode only for dynamic rendering
    if (cb_state.activeRenderPass && cb_state.activeRenderPass->UsesDynamicRendering() &&
        cb_state.HasExternalFormatResolveAttachment()) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
            cb_state.dynamic_state_value.color_blend_enable_attachments.test(0)) {
            const LogObjectList rp_objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
            skip |= LogError(vuid.external_format_resolve_09366, rp_objlist, loc,
                             "blend enable for attachment zero was set to VK_TRUE.");
        }
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
            cb_state.dynamic_state_value.rasterization_samples != VK_SAMPLE_COUNT_1_BIT) {
            const LogObjectList rp_objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
            skip |= LogError(vuid.external_format_resolve_09367, rp_objlist, loc, "rasterization samples set to %s.",
                             string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
        }
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR)) {
            if (cb_state.dynamic_state_value.fragment_size.width != 1) {
                const LogObjectList rp_objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.external_format_resolve_09370, rp_objlist, loc, "fragment size width is %" PRIu32 ".",
                                 cb_state.dynamic_state_value.fragment_size.width);
            }
            if (cb_state.dynamic_state_value.fragment_size.height != 1) {
                const LogObjectList rp_objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.external_format_resolve_09371, rp_objlist, loc, "fragment size height is %" PRIu32 ".",
                                 cb_state.dynamic_state_value.fragment_size.height);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateRayTracingDynamicStateSetStatus(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const vvl::Pipeline& pipeline = *last_bound_state.pipeline_state;
    const vvl::DrawDispatchVuid& vuid = vvl::GetDrawDispatchVuid(loc.function);

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
        if (!cb_state.dynamic_state_status.rtx_stack_size_cb) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(vuid.ray_tracing_pipeline_stack_size_09458, objlist, loc,
                             "VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR state is dynamic, but the command buffer never "
                             "called vkCmdSetRayTracingPipelineStackSizeKHR().");
        }
    } else {
        if (cb_state.dynamic_state_status.rtx_stack_size_pipeline) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(
                vuid.dynamic_state_setting_commands_08608, objlist, loc,
                "%s doesn't set up VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR,  but since the vkCmdBindPipeline, the "
                "related dynamic state commands (vkCmdSetRayTracingPipelineStackSizeKHR) have been called in this command buffer.",
                FormatHandle(pipeline).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ForbidInheritedViewportScissor(const vvl::CommandBuffer& cb_state, const char* vuid, const Location& loc) const {
    bool skip = false;
    if (cb_state.inheritedViewportDepths.size() != 0) {
        skip |= LogError(vuid, cb_state.Handle(), loc,
                         "commandBuffer must not have VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D enabled.");
    }
    return skip;
}

// Used for all vkCmdSet* functions
// Some calls are behind a feature bit that needs to be enabled
bool CoreChecks::ValidateExtendedDynamicState(const vvl::CommandBuffer& cb_state, const Location& loc, bool feature,
                                              const char* vuid, const char* feature_name) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, loc);

    if (!feature) {
        skip |= LogError(vuid, cb_state.Handle(), loc, " %s feature is not enabled.", feature_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport* pViewports, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewport-commandBuffer-04821", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissor-viewportScissor2D-04789", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.exclusiveScissor,
                                        "VUID-vkCmdSetExclusiveScissorNV-None-02031", "exclusiveScissor");
}

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.shadingRateImage,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-None-02064", "shadingRateImage");

    for (uint32_t i = 0; i < viewportCount; ++i) {
        auto *palette = &pShadingRatePalettes[i];
        if (palette->shadingRatePaletteEntryCount == 0 ||
            palette->shadingRatePaletteEntryCount > phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize) {
            skip |=
                LogError("VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071", commandBuffer,
                         error_obj.location.dot(Field::pShadingRatePalettes, i).dot(Field::shadingRatePaletteEntryCount),
                         "(%" PRIu32 ") must be between 1 and shadingRatePaletteSize (%" PRIu32 ").",
                         palette->shadingRatePaletteEntryCount, phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const ErrorObject& error_obj) const {
    return PreCallValidateCmdSetLineStippleKHR(commandBuffer, lineStippleFactor, lineStipplePattern, error_obj);
}

bool CoreChecks::PreCallValidateCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    if ((depthBiasClamp != 0.0) && !enabled_features.depthBiasClamp) {
        skip |=
            LogError("VUID-vkCmdSetDepthBias-depthBiasClamp-00790", commandBuffer, error_obj.location.dot(Field::depthBiasClamp),
                     "is %f, but the depthBiasClamp device feature was not enabled.", depthBiasClamp);
    }
    return skip;
}

bool CoreChecks::ValidateDepthBiasRepresentationInfo(const Location& loc, const LogObjectList& objlist,
                                                     const VkDepthBiasRepresentationInfoEXT& depth_bias_representation) const {
    bool skip = false;

    if ((depth_bias_representation.depthBiasRepresentation ==
         VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT) &&
        !enabled_features.leastRepresentableValueForceUnormRepresentation) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-leastRepresentableValueForceUnormRepresentation-08947", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasRepresentation),
                         "is %s, but the leastRepresentableValueForceUnormRepresentation feature was not enabled.",
                         string_VkDepthBiasRepresentationEXT(depth_bias_representation.depthBiasRepresentation));
    }

    if ((depth_bias_representation.depthBiasRepresentation == VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT) &&
        !enabled_features.floatRepresentation) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-floatRepresentation-08948", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasRepresentation),
                         "is %s but the floatRepresentation feature was not enabled.",
                         string_VkDepthBiasRepresentationEXT(depth_bias_representation.depthBiasRepresentation));
    }

    if ((depth_bias_representation.depthBiasExact == VK_TRUE) && !enabled_features.depthBiasExact) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-depthBiasExact-08949", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasExact),
                         "is VK_TRUE, but the depthBiasExact feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;

    if ((pDepthBiasInfo->depthBiasClamp != 0.0) && !enabled_features.depthBiasClamp) {
        skip |= LogError("VUID-VkDepthBiasInfoEXT-depthBiasClamp-08950", commandBuffer,
                         error_obj.location.dot(Field::pDepthBiasInfo).dot(Field::depthBiasClamp),
                         "is %f, but the depthBiasClamp device feature was not enabled.", pDepthBiasInfo->depthBiasClamp);
    }

    if (const auto *depth_bias_representation = vku::FindStructInPNextChain<VkDepthBiasRepresentationInfoEXT>(pDepthBiasInfo->pNext)) {
        skip |= ValidateDepthBiasRepresentationInfo(error_obj.location, error_obj.objlist, *depth_bias_representation);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                                  const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);

    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
            skip |= LogError(
                "VUID-vkCmdSetDepthBounds-minDepthBounds-00600", commandBuffer, error_obj.location.dot(Field::minDepthBounds),
                "is %f which is not within the [0.0, 1.0] range and VK_EXT_depth_range_unrestricted extension was not enabled.",
                minDepthBounds);
        }

        if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
            skip |= LogError(
                "VUID-vkCmdSetDepthBounds-maxDepthBounds-00601", commandBuffer, error_obj.location.dot(Field::maxDepthBounds),
                "is %f which is not within the [0.0, 1.0] range and VK_EXT_depth_range_unrestricted extension was not enabled.",
                maxDepthBounds);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |=
        ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetDiscardRectangleEXT-viewportScissor2D-04788", error_obj.location);
    for (uint32_t i = 0; i < discardRectangleCount; ++i) {
        if (pDiscardRectangles[i].offset.x < 0) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-x-00587", commandBuffer,
                             error_obj.location.dot(Field::pDiscardRectangles, i).dot(Field::offset).dot(Field::x),
                             "(%" PRId32 ") is negative.", pDiscardRectangles[i].offset.x);
        }
        if (pDiscardRectangles[i].offset.y < 0) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-x-00587", commandBuffer,
                             error_obj.location.dot(Field::pDiscardRectangles, i).dot(Field::offset).dot(Field::y),
                             "(%" PRId32 ") is negative.", pDiscardRectangles[i].offset.y);
        }
    }
    if (firstDiscardRectangle + discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
        skip |=
            LogError("VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585", commandBuffer,
                     error_obj.location.dot(Field::firstDiscardRectangle),
                     "(%" PRIu32 ") + discardRectangleCount (%" PRIu32 ") is not less than maxDiscardRectangles (%" PRIu32 ").",
                     firstDiscardRectangle, discardRectangleCount, phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |= ValidateSampleLocationsInfo(*pSampleLocationsInfo, error_obj.location.dot(Field::pSampleLocationsInfo));

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                                 const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState2LogicOp || enabled_features.shaderObject,
                                        "VUID-vkCmdSetLogicOpEXT-None-09422", "extendedDynamicState2LogicOp or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState2PatchControlPoints || enabled_features.shaderObject,
        "VUID-vkCmdSetPatchControlPointsEXT-None-09422", "extendedDynamicState2PatchControlPoints or shaderObject");

    if (patchControlPoints > phys_dev_props.limits.maxTessellationPatchSize) {
        skip |= LogError("VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874", commandBuffer,
                         error_obj.location.dot(Field::patchControlPoints),
                         "(%" PRIu32
                         ") must be less than "
                         "maxTessellationPatchSize (%" PRIu32 ")",
                         patchControlPoints, phys_dev_props.limits.maxTessellationPatchSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                 const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState2 || enabled_features.shaderObject,
                                        "VUID-vkCmdSetRasterizerDiscardEnable-None-08970", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState2 || enabled_features.shaderObject,
                                        "VUID-vkCmdSetDepthBiasEnable-None-08970", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState2 || enabled_features.shaderObject,
                                        "VUID-vkCmdSetPrimitiveRestartEnable-None-08970", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                  const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetCullMode-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetFrontFace-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetPrimitiveTopology-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport* pViewports, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                         enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                         "VUID-vkCmdSetViewportWithCount-None-08971", "extendedDynamicState or shaderObject");
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewportWithCount-commandBuffer-04819", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport* pViewports, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewportWithCount-commandBuffer-04819", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                         enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                         "VUID-vkCmdSetScissorWithCount-None-08971", "extendedDynamicState or shaderObject");
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissorWithCount-commandBuffer-04820", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissorWithCount-commandBuffer-04820", error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetDepthTestEnable-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetDepthWriteEnable-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                       const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetDepthCompareOp-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetDepthBoundsTestEnable-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetStencilTestEnable-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetStencilOp-None-08971", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin,
                                                                  const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3TessellationDomainOrigin || enabled_features.shaderObject,
        "VUID-vkCmdSetTessellationDomainOriginEXT-None-09423", "extendedDynamicState3TessellationDomainOrigin or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3DepthClampEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetDepthClampEnableEXT-None-09423", "extendedDynamicState3DepthClampEnable or shaderObject");
    if (depthClampEnable != VK_FALSE && !enabled_features.depthClamp) {
        skip |= LogError("VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449", commandBuffer,
                         error_obj.location.dot(Field::depthClampEnable), "is VK_TRUE but the depthClamp feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3PolygonMode || enabled_features.shaderObject,
        "VUID-vkCmdSetPolygonModeEXT-None-09423", "extendedDynamicState3PolygonMode or shaderObject");
    if ((polygonMode == VK_POLYGON_MODE_LINE || polygonMode == VK_POLYGON_MODE_POINT) && !enabled_features.fillModeNonSolid) {
        skip |= LogError("VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424", commandBuffer,
                         error_obj.location.dot(Field::polygonMode),
                         "is %s but the "
                         "fillModeNonSolid feature was not enabled.",
                         string_VkPolygonMode(polygonMode));
    } else if (polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV && !IsExtEnabled(device_extensions.vk_nv_fill_rectangle)) {
        skip |= LogError("VUID-vkCmdSetPolygonModeEXT-polygonMode-07425", commandBuffer, error_obj.location.dot(Field::polygonMode),
                         "is VK_POLYGON_MODE_FILL_RECTANGLE_NV but the VK_NV_fill_rectangle "
                         "extension was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3RasterizationSamples || enabled_features.shaderObject,
        "VUID-vkCmdSetRasterizationSamplesEXT-None-09423", "extendedDynamicState3RasterizationSamples or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask* pSampleMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.extendedDynamicState3SampleMask || enabled_features.shaderObject,
                                        "VUID-vkCmdSetSampleMaskEXT-None-09423", "extendedDynamicState3SampleMask or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3AlphaToCoverageEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetAlphaToCoverageEnableEXT-None-09423", "extendedDynamicState3AlphaToCoverageEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3AlphaToOneEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetAlphaToOneEnableEXT-None-09423", "extendedDynamicState3AlphaToOneEnable or shaderObject");
    if (alphaToOneEnable != VK_FALSE && !enabled_features.alphaToOne) {
        skip |= LogError("VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607", commandBuffer,
                         error_obj.location.dot(Field::alphaToOneEnable), "is VK_TRUE but the alphaToOne feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                       const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3LogicOpEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetLogicOpEnableEXT-None-09423", "extendedDynamicState3LogicOpEnable or shaderObject");
    if (logicOpEnable != VK_FALSE && !enabled_features.logicOp) {
        skip |= LogError("VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366", commandBuffer, error_obj.location.dot(Field::logicOpEnable),
                         "is VK_TRUE but the logicOp feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ColorBlendEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetColorBlendEnableEXT-None-09423", "extendedDynamicState3ColorBlendEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT* pColorBlendEquations,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ColorBlendEquation || enabled_features.shaderObject,
        "VUID-vkCmdSetColorBlendEquationEXT-None-09423", "extendedDynamicState3ColorBlendEquation or shaderObject");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        const Location equation_loc = error_obj.location.dot(Field::pColorBlendEquations, attachment);
        VkColorBlendEquationEXT const &equation = pColorBlendEquations[attachment];
        if (!enabled_features.dualSrcBlend) {
            if (IsSecondaryColorInputBlendFactor(equation.srcColorBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357", commandBuffer, equation_loc.dot(Field::srcColorBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstColorBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358", commandBuffer, equation_loc.dot(Field::dstColorBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.dstColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.srcAlphaBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359", commandBuffer, equation_loc.dot(Field::srcAlphaBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.srcAlphaBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstAlphaBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360", commandBuffer, equation_loc.dot(Field::dstAlphaBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.dstAlphaBlendFactor));
            }
        }
        if (IsAdvanceBlendOperation(equation.colorBlendOp) || IsAdvanceBlendOperation(equation.alphaBlendOp)) {
            skip |=
                LogError("VUID-VkColorBlendEquationEXT-colorBlendOp-07361", commandBuffer, equation_loc.dot(Field::colorBlendOp),
                         "(%s) and alphaBlendOp (%s) must not be an advanced blending operation.",
                         string_VkBlendOp(equation.colorBlendOp), string_VkBlendOp(equation.alphaBlendOp));
        }
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset) && !enabled_features.constantAlphaColorBlendFactors) {
            if (equation.srcColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07362", commandBuffer,
                                 equation_loc.dot(Field::srcColorBlendFactor),
                                 "is %s but the constantAlphaColorBlendFactors feature was not supported.",
                                 string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (equation.dstColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07363", commandBuffer,
                                 equation_loc.dot(Field::dstColorBlendFactor),
                                 "is %s but the constantAlphaColorBlendFactors feature was not supported.",
                                 string_VkBlendFactor(equation.dstColorBlendFactor));
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ColorWriteMask || enabled_features.shaderObject,
        "VUID-vkCmdSetColorWriteMaskEXT-None-09423", "extendedDynamicState3ColorWriteMask or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3RasterizationStream || enabled_features.shaderObject,
        "VUID-vkCmdSetRasterizationStreamEXT-None-09423", "extendedDynamicState3RasterizationStream or shaderObject");
    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411", commandBuffer, error_obj.location,
                         "the transformFeedback feature was not enabled.");
    }
    if (rasterizationStream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412", commandBuffer,
                         error_obj.location.dot(Field::rasterizationStream),
                         "(%" PRIu32 ") must be less than maxTransformFeedbackStreams (%" PRIu32 ").", rasterizationStream,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
    }
    if (rasterizationStream != 0U &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413", commandBuffer,
                         error_obj.location.dot(Field::rasterizationStream),
                         "(%" PRIu32
                         ") is non-zero but "
                         "the transformFeedbackRasterizationStreamSelect feature was not supported.",
                         rasterizationStream);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3ConservativeRasterizationMode || enabled_features.shaderObject,
        "VUID-vkCmdSetConservativeRasterizationModeEXT-None-09423",
        "extendedDynamicState3ConservativeRasterizationMode or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize,
                                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3ExtraPrimitiveOverestimationSize || enabled_features.shaderObject,
        "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-None-09423",
        "extendedDynamicState3ExtraPrimitiveOverestimationSize or shaderObject");
    if (extraPrimitiveOverestimationSize < 0.0f ||
        extraPrimitiveOverestimationSize >
            phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
        skip |= LogError("VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428", commandBuffer,
                         error_obj.location.dot(Field::extraPrimitiveOverestimationSize),
                         "(%f) must be less then zero or greater than maxExtraPrimitiveOverestimationSize (%f).",
                         extraPrimitiveOverestimationSize,
                         phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3DepthClipEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetDepthClipEnableEXT-None-09423", "extendedDynamicState3DepthClipEnable or shaderObject");
    if (!enabled_features.depthClipEnable) {
        skip |= LogError("VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451", commandBuffer, error_obj.location,
                         "the depthClipEnable feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3SampleLocationsEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetSampleLocationsEnableEXT-None-09423", "extendedDynamicState3SampleLocationsEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ColorBlendAdvanced || enabled_features.shaderObject,
        "VUID-vkCmdSetColorBlendAdvancedEXT-None-09423", "extendedDynamicState3ColorBlendAdvanced or shaderObject");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendAdvancedEXT const &advanced = pColorBlendAdvanced[attachment];
        if (advanced.srcPremultiplied == VK_TRUE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::srcPremultiplied),
                             "is VK_TRUE but the advancedBlendNonPremultipliedSrcColor feature was not enabled.");
        }
        if (advanced.dstPremultiplied == VK_TRUE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::dstPremultiplied),
                             "is VK_TRUE but the advancedBlendNonPremultipliedDstColor feature was not enabled.");
        }
        if (advanced.blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-blendOverlap-07507", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::blendOverlap),
                             "is %s, but the advancedBlendCorrelatedOverlap feature was not supported.",
                             string_VkBlendOverlapEXT(advanced.blendOverlap));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ProvokingVertexMode || enabled_features.shaderObject,
        "VUID-vkCmdSetProvokingVertexModeEXT-None-09423", "extendedDynamicState3ProvokingVertexMode or shaderObject");
    if (provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT && enabled_features.provokingVertexLast == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447", commandBuffer,
                         error_obj.location.dot(Field::provokingVertexMode),
                         "is VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but the provokingVertexLast feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3LineRasterizationMode || enabled_features.shaderObject,
        "VUID-vkCmdSetLineRasterizationModeEXT-None-09423", "extendedDynamicState3LineRasterizationMode or shaderObject");
    if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_KHR && !enabled_features.rectangularLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR_KHR "
                         "but the rectangularLines feature was not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_KHR && !enabled_features.bresenhamLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is VK_LINE_RASTERIZATION_MODE_BRESENHAM_KHR "
                         "but the bresenhamLines feature was not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR && !enabled_features.smoothLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is "
                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR but the smoothLines feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3LineStippleEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetLineStippleEnableEXT-None-09423", "extendedDynamicState3LineStippleEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3DepthClipNegativeOneToOne || enabled_features.shaderObject,
        "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-None-09423", "extendedDynamicState3DepthClipNegativeOneToOne or shaderObject");
    if (enabled_features.depthClipControl == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453", commandBuffer, error_obj.location,
                         "the depthClipControl feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3ViewportWScalingEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetViewportWScalingEnableNV-None-09423", "extendedDynamicState3ViewportWScalingEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3ViewportSwizzle || enabled_features.shaderObject,
        "VUID-vkCmdSetViewportSwizzleNV-None-09423", "extendedDynamicState3ViewportSwizzle or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3CoverageToColorEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageToColorEnableNV-None-09423", "extendedDynamicState3CoverageToColorEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3CoverageToColorLocation || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageToColorLocationNV-None-09423", "extendedDynamicState3CoverageToColorLocation or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3CoverageModulationMode || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationModeNV-None-09423", "extendedDynamicState3CoverageModulationMode or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable,
                                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3CoverageModulationTableEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationTableEnableNV-None-09423",
        "extendedDynamicState3CoverageModulationTableEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float* pCoverageModulationTable,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3CoverageModulationTable || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationTableNV-None-09423", "extendedDynamicState3CoverageModulationTable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3ShadingRateImageEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetShadingRateImageEnableNV-None-09423", "extendedDynamicState3ShadingRateImageEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable,
                                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.extendedDynamicState3RepresentativeFragmentTestEnable || enabled_features.shaderObject,
        "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-None-09423",
        "extendedDynamicState3RepresentativeFragmentTestEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, error_obj.location, enabled_features.extendedDynamicState3CoverageReductionMode || enabled_features.shaderObject,
        "VUID-vkCmdSetCoverageReductionModeNV-None-09423", "extendedDynamicState3CoverageReductionMode or shaderObject");
}

bool CoreChecks::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                            const ErrorObject& error_obj) const {
    bool skip = false;
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if (VK_FALSE == enabled_features.events) {
            skip |= LogError("VUID-vkCreateEvent-events-04468", device, error_obj.location,
                             "events are not supported via VK_KHR_portability_subset");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, error_obj.location,
        enabled_features.pipelineFragmentShadingRate || enabled_features.primitiveFragmentShadingRate ||
            enabled_features.attachmentFragmentShadingRate,
        "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509",
        "pipelineFragmentShadingRate, primitiveFragmentShadingRate, or attachmentFragmentShadingRate");

    if (!enabled_features.pipelineFragmentShadingRate && pFragmentSize->width != 1) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width),
                         "is %" PRIu32 " but the pipelineFragmentShadingRate feature was not enabled.", pFragmentSize->width);
    }

    if (!enabled_features.pipelineFragmentShadingRate && pFragmentSize->height != 1) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height),
                         "is %" PRIu32 " but the pipelineFragmentShadingRate feature was not enabled.", pFragmentSize->height);
    }

    if (!enabled_features.primitiveFragmentShadingRate && combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |=
            LogError("VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510", commandBuffer,
                     error_obj.location.dot(Field::combinerOps, 0), "is %s but the primitiveFragmentShadingRate was not enabled.",
                     string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]));
    }

    if (!enabled_features.attachmentFragmentShadingRate && combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |=
            LogError("VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511", commandBuffer,
                     error_obj.location.dot(Field::combinerOps, 1), "is %s but the attachmentFragmentShadingRate was not enabled.",
                     string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512", commandBuffer,
                         error_obj.location.dot(Field::combinerOps, 0),
                         "is %s but the fragmentShadingRateNonTrivialCombinerOps feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512", commandBuffer,
                         error_obj.location.dot(Field::combinerOps, 1),
                         "is %s but the fragmentShadingRateNonTrivialCombinerOps feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]));
    }

    if (pFragmentSize->width == 0) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "is zero");
    }

    if (pFragmentSize->height == 0) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "is zero");
    }

    if (pFragmentSize->width != 0 && !IsPowerOfTwo(pFragmentSize->width)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "(%" PRIu32 ") is a non-power-of-two.",
                         pFragmentSize->width);
    }

    if (pFragmentSize->height != 0 && !IsPowerOfTwo(pFragmentSize->height)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "(%" PRIu32 ") is a non-power-of-two.",
                         pFragmentSize->height);
    }

    if (pFragmentSize->width > 4) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "(%" PRIu32 ") is larger than 4.",
                         pFragmentSize->width);
    }

    if (pFragmentSize->height > 4) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "(%" PRIu32 ") is larger than 4.",
                         pFragmentSize->height);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32* pColorWriteEnables, const ErrorObject& error_obj) const {
    bool skip = false;

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.colorWriteEnable,
                                         "VUID-vkCmdSetColorWriteEnableEXT-None-04803", "colorWriteEnable");

    if (attachmentCount > phys_dev_props.limits.maxColorAttachments) {
        skip |= LogError("VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656", commandBuffer,
                         error_obj.location.dot(Field::attachmentCount),
                         "(%" PRIu32 ") is greater than the maxColorAttachments limit (%" PRIu32 ").", attachmentCount,
                         phys_dev_props.limits.maxColorAttachments);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                     const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                     uint32_t vertexAttributeDescriptionCount,
                                                     const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location,
                                        enabled_features.vertexInputDynamicState || enabled_features.shaderObject,
                                        "VUID-vkCmdSetVertexInputEXT-None-08546", "vertexInputDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.fragmentShadingRateEnums,
                                        "VUID-vkCmdSetFragmentShadingRateEnumNV-fragmentShadingRateEnums-04579",
                                        "fragmentShadingRateEnums");
}

bool CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateExtendedDynamicState(*cb_state, error_obj.location, enabled_features.attachmentFeedbackLoopDynamicState,
                                         "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopDynamicState-08862",
                                         "attachmentFeedbackLoopDynamicState");

    if (aspectMask != VK_IMAGE_ASPECT_NONE && !enabled_features.attachmentFeedbackLoopLayout) {
        skip |= LogError("VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopLayout-08864", commandBuffer,
                         error_obj.location.dot(Field::aspectMask),
                         "is %s but the attachmentFeedbackLoopLayout feature "
                         "was not enabled.",
                         string_VkImageAspectFlags(aspectMask).c_str());
    }

    if ((aspectMask &
         ~(VK_IMAGE_ASPECT_NONE | VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
        skip |= LogError("VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-aspectMask-08863", commandBuffer,
                         error_obj.location.dot(Field::aspectMask), "is %s.", string_VkImageAspectFlags(aspectMask).c_str());
    }

    return skip;
}
