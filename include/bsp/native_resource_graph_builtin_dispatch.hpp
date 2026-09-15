#pragma once
#include "bsp/native_resource_graph_builder.hpp"
#include "bsp/native_model_base_lifetime.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <list>
#include <memory>

namespace bsp {
struct NativeResourceGraphBuiltinEnvironment {
    NativeGroupEnvironment& groups; // same scene, lifetime, type and name domains
    void* actual_plain_pool_0108ff58;
    void* actual_model_base_pool_0109008c;
    NativeNodeRawConstants constants;
    NativeModelBaseTypeTokens model_base_types;
    const volatile std::uint32_t* plain_factory_00d63210; // at least2 words
    const volatile std::uint32_t* model_factory_00d63218; // at least2 words
    const volatile std::uint32_t* group_factory_00d63220; // at least2 words
    const volatile std::uint32_t* model_base_00d62d78; // at least22 words
};

// Production provider for B891A0's three built-in factories. All native pools
// and type descriptors must already be initialized. Owns only the stable C++
// companions; physical owners retain their original reference counts and pools.
// Resource/item/foreign dispatch is forwarded to a complete required provider.
// The context, profiles and serialized owning thread outlive every created node.
class NativeResourceGraphBuiltinDispatch final : public NativeResourceGraphCalls {
public:
    NativeResourceGraphBuiltinDispatch(NativeResourceGraphBuiltinEnvironment,
        NativeResourceGraphCalls& external);
    ~NativeResourceGraphBuiltinDispatch() override;
    NativeResourceGraphBuiltinDispatch(const NativeResourceGraphBuiltinDispatch&) = delete;
    NativeResourceGraphBuiltinDispatch& operator=(const NativeResourceGraphBuiltinDispatch&) = delete;
    NativeStringStorage& names() noexcept { return graph_names_; }
    std::size_t live_owned_nodes() const noexcept;
    const volatile std::uint32_t* table(std::uint32_t profile) noexcept override;
    void* create_instance(std::uint32_t target, void* resource) override;
    void* item_factory(std::uint32_t target, void* item) override;
    void* create_node(std::uint32_t target, void* factory, const void* name) override;
    void set_matrix(std::uint32_t target, void* node, const void* matrix) override;
    std::uint8_t is_type(std::uint32_t target, void* node, std::uint32_t token) override;
    void attach_item(std::uint32_t target, void* item, void* instance,
        void* record, void* node, std::uint32_t creation_word) override;
    NativeNodeBinding& node_binding(void* actual_node) noexcept override;
    NativeGroupOwner& group_owner(void* actual_group) noexcept override;
private:
    struct Record;
    NativeResourceGraphBuiltinEnvironment environment_;
    NativeResourceGraphCalls& external_;
    NativeStringRawPoolContext& raw_names_;
    ActualNativeStringPoolStorage graph_names_;
    std::list<std::unique_ptr<Record>> records_; // companion ownership, no hierarchy copy
    void retire(Record&) noexcept;
};
} // namespace bsp
