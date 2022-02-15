#pragma once

#include <vector>
#include <memory>
#include "base_node.h"

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayout;
class DescriptorSetLayoutDef;
}  // namespace cvdescriptorset

class ValidationStateTracker;

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Defines/stores a compatibility defintion for set N
// The "layout layout" must store at least set+1 entries, but only the first set+1 are considered for hash and equality testing
// Note: the "cannonical" data are referenced by Id, not including handle or device specific state
// Note: hash and equality only consider layout_id entries [0, set] for determining uniqueness
struct PipelineLayoutCompatDef {
    uint32_t set;
    PushConstantRangesId push_constant_ranges;
    PipelineLayoutSetLayoutsId set_layouts_id;
    PipelineLayoutCompatDef(const uint32_t set_index, const PushConstantRangesId pcr_id, const PipelineLayoutSetLayoutsId sl_id)
        : set(set_index), push_constant_ranges(pcr_id), set_layouts_id(sl_id) {}
    size_t hash() const;
    bool operator==(const PipelineLayoutCompatDef &other) const;
};

// Canonical dictionary for PipelineLayoutCompat records
using PipelineLayoutCompatDict = hash_util::Dictionary<PipelineLayoutCompatDef, hash_util::HasHashMember<PipelineLayoutCompatDef>>;
using PipelineLayoutCompatId = PipelineLayoutCompatDict::Id;

// Store layouts and pushconstants for PipelineLayout
class PIPELINE_LAYOUT_STATE : public BASE_NODE {
  public:
    using SetLayoutVector = std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    // canonical form IDs for the "compatible for set" contents
    const PushConstantRangesId push_constant_ranges;
    // table of "compatible for set N" cannonical forms for trivial accept validation
    const std::vector<PipelineLayoutCompatId> compat_for_set;

    PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l, const VkPipelineLayoutCreateInfo *pCreateInfo);

    VkPipelineLayout layout() const { return handle_.Cast<VkPipelineLayout>(); }

    std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> GetDsl(uint32_t set) const {
        std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> dsl = nullptr;
        if (set < set_layouts.size()) {
            dsl = set_layouts[set];
        }
        return dsl;
    }
};
