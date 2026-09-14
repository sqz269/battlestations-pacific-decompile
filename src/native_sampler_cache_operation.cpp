#include "bsp/native_sampler_cache_operation.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Op = NativeSamplerCacheOperation;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
void* at(const void* p, U offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset);
}
U word(const void* p, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(p, offset));
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
void* pointer(const void* p, U offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
void clear_name(void* p) noexcept { put(p, 0, 0); put(p, 4, 0); }
void copy_name(void* to, const void* from, Op& a, U resize_site, U copy_site) {
    if (to == from) return;
    a.native_site = resize_site;
    resize_native_string_header_0041dd40(to, a.context->strings, word(from), true);
    if (word(from) != 0) {
        const U count = word(to);
        const void* input = pointer(from, 4);
        void* output = pointer(to, 4);
        a.native_site = copy_site;
        if (count) std::memmove(output, input, count);
    }
}
void release_name(void* p, bool& live, Op& a, U site) {
    auto* data = static_cast<char*>(pointer(p, 4)); // Before disarming.
    live = false;
    if (data) {
        const U bytes = word(p) + 1u;
        a.native_site = site;
        a.context->strings.release(data, bytes);
    }
}
U selector(Op& a, U offset) {
    if (word(a.actual_cache) != 0x00ce7d24)
        throw std::invalid_argument("Unimplemented actual sampler cache profile");
    return word(a.context->actual_secondary_profile_00ce7d24, offset);
}
void invalid(Op& a, U site) {
    a.native_site = site;
    const auto& c = a.context->validation;
    c.invalid_parameter(c.context);
}
bool alias_equal(const void* alias_name, const void* name, Op& a, U site) {
    const U left = word(alias_name), right = word(name);
    if (left != right) return false;
    if (!left) return right == 0;
    if (!right) return false;
    const auto* rhs = static_cast<const char*>(pointer(name, 4));
    const auto* lhs = static_cast<const char*>(pointer(alias_name, 4));
    a.native_site = site;
    return _stricmp(lhs, rhs) == 0;
}
bool unequal(const void* left, const void* right, Op& a, U site) {
    if (!word(left)) return word(right) != 0;
    if (!word(right)) return true;
    // Native callers fetch right data, then left data, before cdecl pushes.
    const auto* rhs = static_cast<const char*>(pointer(right, 4));
    const auto* lhs = static_cast<const char*>(pointer(left, 4));
    a.native_site = site;
    return _stricmp(lhs, rhs) != 0;
}
void append_alias(void* list, const void* name, Op& a, U allocate_site, U count_site) {
    auto* sentinel = static_cast<NativeRenderResourceAliasNode*>(pointer(list, 4));
    auto* previous = static_cast<NativeRenderResourceAliasNode*>(pointer(sentinel, 4));
    void* const captured_link = at(sentinel, 4);
    a.native_site = allocate_site;
    a.pending_alias = allocate_native_render_alias_node_004ce6f0(sentinel, previous, name, a.context->strings);
    a.native_site = count_site;
    grow_native_alias_list_count_004ce780(list, 1);
    put(captured_link, 0, reinterpret_cast<U>(a.pending_alias));
    void* const prior = pointer(a.pending_alias, 4);
    put(prior, 0, reinterpret_cast<U>(a.pending_alias));
    a.pending_alias = nullptr;
}
void* retain(Op& a, void* resource, U site) {
    a.native_site = site;
    if (selector(a, 0xc) != 0x004ddb20)
        throw std::invalid_argument("Unimplemented actual sampler cache retain slot");
    return retain_native_cache_resource_004ddb20(resource);
}
void finish_names(Op& a, U resolved_site, U requested_site) {
    release_name(a.resolved, a.resolved_live, a, resolved_site);
    release_name(a.requested, a.requested_live, a, requested_site);
}
void* run(Op& a, std::uint8_t retain_new, std::uint8_t allow_load) {
    // Original first header read follows successful BECCD0. No entry/pump
    // work is hidden in this continuation or performed through a projection.
    const void* input = a.input_name;
    put(a.hidden_or_resource, 0, 0);
    clear_name(a.requested);
    copy_name(a.requested, input, a, 0xb1a540, 0xb1a557);
    a.requested_live = true;
    a.native_site = 0xb1a567;
    normalize_native_resource_path_header_00bee690(a.requested, a.context->strings);

    const U count = word(a.actual_cache, 8);
    void* record = pointer(a.actual_cache, 4);
    const void* const end = at(record, count * 0x2cu);
    while (record != end) {
        void* const list = at(record, 8);
        void* const sentinel = pointer(list, 4);
        void* node = pointer(sentinel);
        while (node != sentinel) {
            // B1A593 CMP EAX,EAX always skips B1A597. Other CRT checks return.
            if (node == pointer(list, 4)) invalid(a, 0xb1a5a5);
            if (alias_equal(at(node, 8), a.requested, a, 0xb1a5d3)) {
                put(a.hidden_or_resource, 0, word(record, 0x28));
                break;
            }
            if (node == pointer(list, 4)) invalid(a, 0xb1a5e9);
            node = pointer(node);
        }
        if (word(a.hidden_or_resource)) break;
        record = at(record, 0x2c);
    }
    clear_name(a.resolved); a.resolved_live = true;
    if (void* cached = pointer(a.hidden_or_resource)) {
        a.result = retain(a, cached, 0xb1a896);
        finish_names(a, 0xb1a9e7, 0xb1aa0e);
        return a.result;
    }
    a.native_site = 0xb1a649;
    if (selector(a, 4) != 0x00b19e40)
        throw std::invalid_argument("Unimplemented actual sampler cache resolve slot");
    const void* returned = copy_native_cache_requested_name_00b19e40(
        a.hidden_or_resource, a.requested, a.options, a.context->strings);
    a.hidden_live = true;
    copy_name(a.resolved, returned, a, 0xb1a663, 0xb1a67a);
    release_name(a.hidden_or_resource, a.hidden_live, a, 0xb1a6a1);
    a.native_site = 0xb1a6aa;
    normalize_native_resource_path_header_00bee690(a.resolved, a.context->strings);
    if (unequal(a.resolved, a.requested, a, 0xb1a6cf)) {
        const U second_count = word(a.actual_cache, 8);
        record = pointer(a.actual_cache, 4);
        const void* const second_end = at(record, second_count * 0x2cu);
        while (record != second_end) {
            void* const list = at(record, 8);
            void* const sentinel = pointer(list, 4);
            void* const node = pointer(sentinel);
            if (node == sentinel) invalid(a, 0xb1a6f8);
            if (alias_equal(at(node, 8), a.resolved, a, 0xb1a726)) {
                append_alias(list, a.requested, a, 0xb1a85d, 0xb1a868);
                void* const cached = pointer(record, 0x28);
                put(a.hidden_or_resource, 0, reinterpret_cast<U>(cached));
                if (cached) {
                    a.result = retain(a, cached, 0xb1a896);
                    finish_names(a, 0xb1a9e7, 0xb1aa0e);
                    return a.result;
                }
                break; // A null first-alias match goes to creation, no later scan.
            }
            record = at(record, 0x2c);
        }
    }
    if (!allow_load) {
        a.result = nullptr;
        finish_names(a, 0xb1a9e7, 0xb1aa0e);
        return nullptr;
    }
    a.native_site = 0xb1a76d;
    if (selector(a, 8) != 0x00b1b810)
        throw std::invalid_argument("Unimplemented actual sampler cache create slot");
    a.factory_child.emplace();
    a.created_resource = create_native_sampler_factory_resource_00b1b810(a.resolved, a.options,
        a.context->actual_manager_01090aa0, a.context->actual_factory_registry_00f8d41c,
        a.context->factories, *a.factory_child);

    clear_name(&a.record); a.record_name_live = true;
    a.native_site = 0xb1a782;
    auto* sentinel = allocate_native_render_alias_sentinel_004c3020();
    put(&a.record, 0xc, reinterpret_cast<U>(sentinel)); put(&a.record, 0x10, 0);
    put(&a.record, 0x24, 0); put(&a.record, 0x20, 0); put(&a.record, 0x1c, 0);
    put(&a.record, 0x18, 0); put(&a.record, 0x14, 0);
    a.record_live = true; a.record_name_live = false;
    copy_name(&a.record, a.resolved, a, 0xb1a7b3, 0xb1a7cd);
    append_alias(at(&a.record, 8), a.resolved, a, 0xb1a7ea, 0xb1a7f7);
    a.native_site = 0xb1a813;
    void* const manager = a.context->dates.physical.manager_0109ceec;
    const void* date = query_native_vfs_file_date_00bdd340(manager, a.date_output, a.resolved, a.context->dates);
    const bool requested_nonempty = word(a.requested) != 0; // B1A818 before date copies.
    for (U offset = 0; offset != 20; offset += 4) put(&a.record, 0x14 + offset, word(date, offset));
    bool different;
    if (!requested_nonempty) different = word(a.resolved) != 0;
    else if (!word(a.resolved)) different = true;
    else {
        const auto* rhs = static_cast<const char*>(pointer(a.resolved, 4));
        const auto* lhs = static_cast<const char*>(pointer(a.requested, 4));
        a.native_site = 0xb1a8b0; different = _stricmp(lhs, rhs) != 0;
    }
    if (different) append_alias(at(&a.record, 8), a.requested, a, 0xb1a8d1, 0xb1a8de);
    put(&a.record, 0x28, reinterpret_cast<U>(a.created_resource));
    a.native_site = 0xb1a8fa; a.append_entered = true;
    append_native_resource_record_00b1a3c0(*static_cast<NativeResourceRecordVectorStorage*>(at(a.actual_cache, 4)),
        &a.record, a.context->strings, a.context->validation);
    a.append_returned = true;
    a.result = a.created_resource;
    if (retain_new && a.created_resource) a.result = retain(a, a.created_resource, 0xb1a915);
    a.record_live = false; // Original disarms full record before normal destruction.
    a.native_site = retain_new && a.created_resource ? 0xb1a922 : 0xb1a95f;
    destroy_native_resource_record_004d45a0(a.record, a.context->strings);
    finish_names(a, retain_new && a.created_resource ? 0xb1a946 : 0xb1a983,
        retain_new && a.created_resource ? 0xb1aa0e : 0xb1a9aa);
    return a.result;
}
} // namespace
NativeSamplerCacheOperation::~NativeSamplerCacheOperation() {
    if (phase == Phase::running || phase == Phase::failed || requested_live || resolved_live ||
        hidden_live || record_name_live || record_live || pending_alias) std::terminate();
}
void NativeSamplerCacheOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::running || requested_live || resolved_live || hidden_live || record_name_live ||
        record_live || pending_alias || (factory_child && (factory_child->phase == NativeSamplerLoaderOperation::Phase::running ||
        factory_child->phase == NativeSamplerLoaderOperation::Phase::failed))) std::terminate();
    phase = Phase::diagnostic_retired;
}
void* continue_native_sampler_cache_after_pump_00b1a51d(void* cache, const void* name,
    const void* options, std::uint8_t retain_new, std::uint8_t allow_load,
    NativeSamplerCacheContext& c, Op& a) {
    if (a.phase != Op::Phase::fresh) throw std::logic_error("sampler cache continuation is one-shot");
    a.actual_cache = cache; a.input_name = name; a.options = options; a.context = &c;
    a.phase = Op::Phase::running; a.native_site = 0xb1a51d;
    try { void* result = run(a, retain_new, allow_load); a.phase = Op::Phase::complete; return result; }
    catch (...) { a.phase = Op::Phase::failed; throw; }
}
} // namespace bsp
