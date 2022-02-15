#pragma once

#include "pipeline_layout_state.h"

// Graphics pipeline sub-state as defined by VK_KHR_graphics_pipeline_library

struct VertexInputState {
    VertexInputState() = default;
    VertexInputState(const safe_VkGraphicsPipelineCreateInfo &create_info);

    safe_VkPipelineVertexInputStateCreateInfo *input_state = nullptr;
    safe_VkPipelineInputAssemblyStateCreateInfo *input_assembly_state = nullptr;

    using VertexBindingVector = std::vector<VkVertexInputBindingDescription>;
    VertexBindingVector binding_descriptions;

    using VertexBindingIndexMap = layer_data::unordered_map<uint32_t, uint32_t>;
    VertexBindingIndexMap binding_to_index_map;

    using VertexAttrVector = std::vector<VkVertexInputAttributeDescription>;
    VertexAttrVector vertex_attribute_descriptions;

    using VertexAttrAlignmentVector = std::vector<VkDeviceSize>;
    VertexAttrAlignmentVector vertex_attribute_alignments;
};

struct PreRasterState {
    PreRasterState() = default;
    PreRasterState(const ValidationStateTracker &dev_data, const safe_VkGraphicsPipelineCreateInfo &create_info);

    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    safe_VkPipelineViewportStateCreateInfo *viewport_state = nullptr;

    safe_VkPipelineRasterizationStateCreateInfo *raster_state = nullptr;

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    std::shared_ptr<const SHADER_MODULE_STATE> tessc_shader, tesse_shader;
    const safe_VkPipelineShaderStageCreateInfo *tessc_shader_ci = nullptr, *tesse_shader_ci = nullptr;
    const safe_VkPipelineTessellationStateCreateInfo *tess_create_info = nullptr;

    std::shared_ptr<const SHADER_MODULE_STATE> vertex_shader, geometry_shader;
    const safe_VkPipelineShaderStageCreateInfo *vertex_shader_ci = nullptr, *geometry_shader_ci = nullptr;
};

std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const safe_VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const safe_VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const safe_VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const safe_VkPipelineShaderStageCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs);
void SetFragmentShaderInfo(const ValidationStateTracker &state_data, const VkGraphicsPipelineCreateInfo &create_info);
void SetFragmentShaderInfo(const ValidationStateTracker &state_data, const safe_VkGraphicsPipelineCreateInfo &create_info);

struct FragmentShaderState {
    FragmentShaderState() = default;
    FragmentShaderState(const ValidationStateTracker &dev_data, VkRenderPass rp, uint32_t subpass, VkPipelineLayout layout);

    template <typename CreateInfo>
    FragmentShaderState(const ValidationStateTracker &dev_data, const CreateInfo &create_info)
        : FragmentShaderState(dev_data, create_info.renderPass, create_info.subpass, create_info.layout) {
        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
        }
        if (create_info.pDepthStencilState) {
            ds_state = ToSafeDepthStencilState(*create_info.pDepthStencilState);
        }
        SetFragmentShaderInfo(*this, dev_data, create_info);
    }

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ms_state;
    std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ds_state;

    std::shared_ptr<const SHADER_MODULE_STATE> fragment_shader;
    std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> fragment_shader_ci;
};

struct FragmentOutputState {
    using AttachmentVector = std::vector<VkPipelineColorBlendAttachmentState>;

    FragmentOutputState() = default;
    FragmentOutputState(const ValidationStateTracker &dev_data, VkRenderPass rp, uint32_t sp);
    // For a graphics library, a "non-safe" create info must be passed in in order for pColorBlendState and pMultisampleState to not
    // get stripped out. If this is a "normal" pipeline, then we want to keep the logic from safe_VkGraphicsPipelineCreateInfo that
    // strips out pointers that should be ignored.
    template <typename CreateInfo>
    FragmentOutputState(const ValidationStateTracker &dev_data, const CreateInfo &create_info)
        : FragmentOutputState(dev_data, create_info.renderPass, create_info.subpass) {
        if (create_info.pColorBlendState) {
            if (create_info.pColorBlendState) {
                color_blend_state = ToSafeColorBlendState(*create_info.pColorBlendState);
            }
            const auto &cbci = *create_info.pColorBlendState;
            if (cbci.attachmentCount) {
                attachments.reserve(cbci.attachmentCount);
                std::copy(cbci.pAttachments, cbci.pAttachments + cbci.attachmentCount, std::back_inserter(attachments));
            }
            blend_constants_enabled = IsBlendConstantsEnabled(attachments);
        }

        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
            sample_location_enabled = IsSampleLocationEnabled(create_info);
        }

        // TODO
        // auto format_ci = LvlFindInChain<VkPipelineRenderingFormatCreateInfoKHR>(gpci->pNext);
    }

    static bool IsBlendConstantsEnabled(const AttachmentVector &attachments);

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> color_blend_state;
    std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ms_state;

    AttachmentVector attachments;

    bool blend_constants_enabled = false;  // Blend constants enabled for any attachments
    bool sample_location_enabled = false;
};
