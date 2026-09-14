#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/native_resource_load_cache.hpp"
#include "bsp/native_resource_cache_lookup.hpp"
#include "bsp/native_resource_cache_insert.hpp"
#include "bsp/native_resource_cache_pair.hpp"
#include "bsp/native_resource_construction.hpp"
#include "bsp/native_resource_reader_owner.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_root_owner.hpp"
#include "bsp/native_resource_root_dispatch.hpp"
#include "bsp/native_resource_hierarchy_parser.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* p, U n = 0) noexcept { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + n); }
U word(const void* p, U n = 0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p, U n, U v) noexcept { *static_cast<volatile U*>(at(p,n)) = v; }
void* pointer(const void* p, U n = 0) noexcept { return reinterpret_cast<void*>(word(p,n)); }
U bits(const void* p) noexcept { return reinterpret_cast<U>(p); }
void return_captured(void* data, U length, NativeStringRawPoolContext& strings) {
    if (!data) return;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, length + 1u,
        strings.actual_small_returns_disabled_01090aa4);
}
void copy_name(void* destination, const void* source, NativeStringRawPoolContext& strings,
    U& site, U resize_site, U copy_site, bool initialize) {
    const bool identical = destination == source;
    if (initialize) { put(destination,0,0); put(destination,4,0); }
    if (identical) return;
    const U length = word(source);
    site = resize_site;
    resize_native_string_header_0041dd40(destination, strings, length, true);
    if (word(source) != 0) {
        const U count = word(destination);
        void* const input = pointer(source,4);
        void* const output = pointer(destination,4);
        site = copy_site;
        std::memmove(output,input,count);
    }
}
} // namespace

struct NativeResourceLoadCacheAcquired::Impl {
    // Native local offsets relative to ESP after its four saved registers.
    // The reused iterator/pair18h and reader's separate nameB4h stay actual.
    alignas(4) unsigned char locals[0xbc];
    U by_value_name[2];
    NativeVfsNameResolutionAcquired resolution;
    NativeResourceLoadCachePhase phase = NativeResourceLoadCachePhase::fresh;
    U site = 0, failure = 0;
    std::int32_t state = -1, failed_state = -1;
    Impl() {} // Keep unused native local storage uninitialized.
    void* local(U offset) noexcept { return locals + offset; }
    void cleanup(NativeResourceLoadCacheContext& context) noexcept {
        auto& reads = context.dispatch.hierarchy.reads;
        try {
            if (state >= 4) { state = 3; destroy_native_resource_cache_insert_pair_00b7e910(local(0x18),reads.strings); }
            if (state >= 3) { state = 2; destroy_native_resource_cache_pair_00b7e8f0(local(0x34),reads.strings); }
            if (state >= 2) { state = 1; release_native_structured_node_handle_00be9ed0(local(0x24),reads.streams); }
            if (state >= 1) { state = 0; destroy_native_structured_resource_reader_00be9f10(local(0x4c),reads.strings,reads.streams); }
            if (state >= 0) { state = -1; destroy_native_string_header_0041dd20(local(0x10),reads.strings); }
        } catch (...) { std::terminate(); }
    }
};
NativeResourceLoadCacheAcquired::NativeResourceLoadCacheAcquired() : impl_(std::make_unique<Impl>()) {}
NativeResourceLoadCacheAcquired::~NativeResourceLoadCacheAcquired() = default;
NativeResourceLoadCachePhase NativeResourceLoadCacheAcquired::phase() const noexcept { return impl_->phase; }
U NativeResourceLoadCacheAcquired::failure_site() const noexcept { return impl_->failure; }
std::int32_t NativeResourceLoadCacheAcquired::native_state_at_failure() const noexcept { return impl_->failed_state; }
const void* NativeResourceLoadCacheAcquired::actual_reader() const noexcept { return impl_->locals + 0x4c; }
const void* NativeResourceLoadCacheAcquired::resolved_name() const noexcept { return impl_->locals + 0x10; }
const NativeVfsNameResolutionAcquired& NativeResourceLoadCacheAcquired::resolution_invocation() const noexcept { return impl_->resolution; }

void* load_and_cache_native_resource_00b80720(void* manager, const void* name,
    void* factory, NativeResourceLoadCacheContext& context, NativeResourceLoadCacheAcquired& acquired) {
    auto& f = *acquired.impl_;
    if (f.phase != NativeResourceLoadCachePhase::fresh)
        throw std::logic_error("Native resource load invocation cannot replay");
    f.phase = NativeResourceLoadCachePhase::running;
    auto& reads = context.dispatch.hierarchy.reads;
    auto& strings = reads.strings;
    const auto& callbacks = context.dispatch.invalid_parameters;
    auto invalid = [&](U site) { f.site = site; callbacks.invalid_parameter(callbacks.context); };
    try {
        void* const tree = at(manager,0x14);
        put(manager,0x20,bits(factory));
        f.site = 0x00b8075d;
        find_native_resource_cache_name_00b7e7b0(tree,f.local(0x18),name,callbacks);
        void* const head = pointer(tree,4);
        void* const iterator_owner = pointer(f.local(0x18));
        if (!iterator_owner || iterator_owner != tree) invalid(0x00b8077a);
        if (pointer(f.local(0x1c)) != head) {
            if (!iterator_owner) invalid(0x00b8078d);
            if (pointer(f.local(0x1c)) == pointer(iterator_owner,4)) invalid(0x00b8079b);
            void* const node = pointer(f.local(0x1c));
            void* const resource = pointer(node,0x14);
            put(manager,0x24,bits(resource));
            f.site = 0x00b807ae;
            InterlockedIncrement(static_cast<volatile LONG*>(at(resource,4)));
            void* const result = pointer(manager,0x24);
            f.phase = NativeResourceLoadCachePhase::complete;
            return result;
        }
        void* stats = context.actual_allocation_stats_0109cefc;
        U target = word(pointer(stats),4);
        f.site = 0x00b807c7;
        const U starting = context.calls.allocation_metric(target,stats);
        void* const current_factory = pointer(manager,0x20);
        target = word(pointer(current_factory),4);
        f.site = 0x00b807d6;
        void* const resource = context.calls.create_resource(target,current_factory,name);
        put(manager,0x24,bits(resource));
        copy_name(f.local(0x10),name,strings,f.site,0x00b807f4,0x00b8080b,true);
        void* vfs = context.actual_vfs_0109ceec;
        f.state = 0; f.site = 0x00b80825;
        (void)resolve_native_vfs_existing_name_00bdf4c0(vfs,f.local(0x10),context.names,f.resolution);
        vfs = context.actual_vfs_0109ceec;
        target = word(pointer(vfs),4);
        f.site = 0x00b8083c;
        void* const stream = context.calls.open_stream(target,vfs,f.local(0x10),2);
        f.site = 0x00b80844;
        construct_native_structured_resource_reader_00bea150(f.local(0x4c),strings,reads.streams);
        f.state = 1; f.site = 0x00b80856;
        assign_native_resource_reader_stream_00bf0430(f.local(0x4c),stream,reads.streams);
        copy_name(f.local(0xb4),f.local(0x10),strings,f.site,0x00b80869,0x00b80889,false);
        // No null check and no owning slot: this drops the captured open result.
        f.site = 0x00b80895;
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream,4))) == 0) {
            void* const table = pointer(stream);
            const U zero_target = word(table);
            f.site = 0x00b808a6;
            reads.streams.source_zero_reference(zero_target,stream,bits(table));
        }
        f.site = 0x00b808b1;
        create_native_resource_root_00bea700(f.local(0x4c),f.local(0x24),reads);
        f.state = 2; f.site = 0x00b808c5;
        dispatch_native_resource_root_00b7f430(manager,f.local(0x24),context.dispatch);
        void* const cache_resource = pointer(manager,0x24);
        copy_name(f.by_value_name,name,strings,f.site,0x00b808eb,0x00b80900,true);
        f.site = 0x00b80910;
        void* const pair = construct_native_resource_cache_pair_00b7f290(
            f.local(0x34),cache_resource,f.by_value_name,strings);
        f.state = 3;
        copy_name(f.local(0x18),pair,strings,f.site,0x00b8093a,0x00b80951,true);
        void* const captured_pair_data = pointer(f.local(0x18),4);
        put(f.local(0x18),8,word(pair,8));
        f.state = 4; f.site = 0x00b80975;
        insert_native_resource_cache_unique_00b803b0(tree,f.local(0x40),f.local(0x18),strings,callbacks);
        f.state = 3; f.site = 0x00b80991;
        return_captured(captured_pair_data,word(f.local(0x18)),strings);
        void* const first_pair_data = pointer(f.local(0x38));
        f.state = 2; f.site = 0x00b809b8;
        return_captured(first_pair_data,word(f.local(0x34)),strings);
        put(manager,0x20,0);
        stats = context.actual_allocation_stats_0109cefc;
        target = word(pointer(stats),4);
        f.site = 0x00b809d2;
        const U ending = context.calls.allocation_metric(target,stats);
        put(pointer(manager,0x24),0x40,starting - ending);
        void* const result = pointer(manager,0x24);
        f.state = 1; f.site = 0x00b809ef;
        release_native_structured_node_handle_00be9ed0(f.local(0x24),reads.streams);
        f.state = 0; f.site = 0x00b809ff;
        destroy_native_structured_resource_reader_00be9f10(f.local(0x4c),strings,reads.streams);
        void* const resolved_data = pointer(f.local(0x14));
        f.state = -1; f.site = 0x00b80a22;
        return_captured(resolved_data,word(f.local(0x10)),strings);
        f.phase = NativeResourceLoadCachePhase::complete;
        return result;
    } catch (...) {
        f.failure = f.site; f.failed_state = f.state;
        f.cleanup(context);
        f.phase = NativeResourceLoadCachePhase::failed;
        throw;
    }
}

void destroy_native_resource_cache_pair_00b7e8f0(void* pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(pair,strings);
}
void destroy_native_resource_cache_insert_pair_00b7e910(void* pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(pair,strings);
}
U read_native_resource_allocation_budget_00be2700() noexcept {
    U scratch[13];
    std::memset(scratch,0,sizeof(scratch));
    return 0x40000000u - scratch[7];
}
NativeDefaultResourceLoadCacheCalls::NativeDefaultResourceLoadCacheCalls(
    NativeResourceLoadCacheCalls& other, NativeStringRawPoolContext& strings, NativeVfsRuntimeBindings& vfs)
    : other_(other), strings_(strings), vfs_(vfs) {}
U NativeDefaultResourceLoadCacheCalls::allocation_metric(std::uintptr_t target, void* owner) {
    if (target == 0x00be2700) return read_native_resource_allocation_budget_00be2700();
    return other_.allocation_metric(target,owner);
}
void* NativeDefaultResourceLoadCacheCalls::create_resource(std::uintptr_t target, void* factory, const void* name) {
    if (target == 0x00b88340) return create_native_default_resource_00b88340(name,strings_);
    if (target == 0x0071b870) return create_native_game_resource_0071b870(name,strings_);
    return other_.create_resource(target,factory,name);
}
void* NativeDefaultResourceLoadCacheCalls::open_stream(std::uintptr_t target, void* manager,
    const void* name, U flags) { return vfs_.open_manager_entry(target,manager,name,flags); }
} // namespace bsp
