#include "bsp/native_scene_resource_storage.hpp"
#include "bsp/native_ambient_construction.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(at(p, offset)) = value;
}
void* pointer(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(read(p, offset));
}
void terminal_if_zero(void* owner, long count,
    NativeSceneResourceStorageContext& context) {
    if (count == 0) {
        auto& reference = context.ambient.owners.resolve_actual(owner);
        if (static_cast<void*>(&reference.reference_count) != at(owner, 4))
            throw std::logic_error("scene resource terminal requires the captured owner's actual count");
        reference.release_zero_references();
    }
}
void unwind_prefix(void* owner, int state,
    NativeSceneResourceStorageContext& context) noexcept {
    try {
        if (state >= 2)
            destroy_native_scene_registry_00b82e90(at(owner, 0x14), context.registry_storage);
        if (state >= 1)
            destroy_native_string_header_0041dd20(at(owner, 8), context.strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(owner);
    } catch (...) { std::terminate(); }
}
void require_slot(Word current, NativeSceneResourceIdentityContext& context,
    unsigned slot, Word target) {
    if (current != 0x00d63168u || !context.resource_profile_00d63168 ||
        context.resource_profile_00d63168[slot] != target)
        throw std::logic_error("scene resource has no binding for the current native slot");
}
std::atomic<std::int32_t>& bind_count(void* owner,
    NativeSceneResourceIdentityContext& context) {
    if (!owner || reinterpret_cast<Word>(owner) % 4 || context.storage.ambient.owners.find(owner))
        throw std::invalid_argument("scene resource requires aligned unregistered actual storage");
    require_slot(read(owner), context, 0, 0x00bd30e0u);
    require_slot(read(owner), context, 1, 0x00b83410u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("scene resource requires a completed live actual count");
    return count;
}
class ResourceDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    ResourceDeleteCalls(void* owner, NativeSceneResourceReference& reference,
        NativeSceneResourceIdentityContext& context)
        : owner_(owner), reference_(reference), context_(context) {}
    void delete_vslot04(void* owner, Word captured_profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("scene resource terminal requires same identity and flags1");
        require_slot(captured_profile, context_, 1, 0x00b83410u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b83410(actual_flags);
    }
private:
    void* const owner_;
    NativeSceneResourceReference& reference_;
    NativeSceneResourceIdentityContext& context_;
};
}

void* construct_native_scene_resource_00b83c50(void* owner,
    NativeSceneResourceConstructFrame& frame, NativeSceneResourceStorageContext& context,
    NativeSceneResourceAcquired& acquired) {
    if (acquired.started || acquired.ambient_reference ||
        &context.ambient.owners != &context.ambient_identity.registry)
        throw std::invalid_argument("scene constructor needs fresh diagnostics and one actual owner registry");
    acquired.started = true;
    int state = -1;
    try {
        write(owner, 0, 0x00ceb130u);
        write(owner, 4, 1);
        const void* const name = reinterpret_cast<const void*>(frame.name_or_ambient_argument);
        void* const header = at(owner, 8);
        const bool self = header == name; // comparison precedes header zero stores
        write(owner, 0, 0x00d63168u);
        state = acquired.native_eh_state = 0;
        write(header, 0, 0);
        write(header, 4, 0);
        if (!self) {
            const Word length = read(name);
            acquired.active_call_site = 0x00b83cacu;
            resize_native_string_header_0041dd40(header, context.strings, length, true);
            if (read(name) != 0) {
                const Word copied = read(header);
                void* const destination = pointer(header, 4);
                const void* const source = pointer(name, 4);
                acquired.active_call_site = 0x00b83cc2u;
                if (copied != 0) std::memmove(destination, source, copied);
            }
        }
        state = acquired.native_eh_state = 1;
        write(owner, 0x10, 0);
        acquired.active_call_site = 0x00b83ce3u;
        construct_native_scene_registry_00b83600(at(owner, 0x14),
            &frame.allocator_first, &frame.allocator_second, frame.registry,
            context.registry_storage);
        state = acquired.native_eh_state = 2;
        acquired.active_call_site = 0x00b83cf2u;
        void* const allocation = context.registry_storage.actual_00bf681b(0x98);
        frame.name_or_ambient_argument = reinterpret_cast<Word>(allocation);
        acquired.raw_ambient = allocation;
        state = acquired.native_eh_state = 3;
        void* creator = nullptr;
        if (allocation) {
            acquired.active_call_site = 0x00b83d09u;
            creator = construct_native_ambient_00b7c290(allocation, context.one_00d7a24c);
        }
        acquired.ambient_creator = creator;
        state = acquired.native_eh_state = 2;
        if (creator) {
            // Explicit host metadata admission, not a recovered native call.
            // No native stores/credits; failure leaves completed raw creator.
            try { acquired.ambient_reference.emplace(creator, context.ambient_identity); }
            catch (...) { acquired.failure = NativeSceneResourceAcquired::Failure::host_metadata_admission; throw; }
        }
        frame.requested_ambient_argument = reinterpret_cast<Word>(creator);
        acquired.active_call_site = 0x00b83d1cu;
        set_native_scene_resource_ambient_00b825d0(owner,
            frame.requested_ambient_argument, frame.setter, context.ambient);
        acquired.active_call_site = 0x00b83d25u;
        const long count = context.ambient.decrement_00ce2220(
            static_cast<volatile long*>(at(creator, 4)));
        acquired.creator_decrement_completed = true;
        if (count == 0) acquired.active_call_site = 0x00b83d35u;
        terminal_if_zero(creator, count, context);
        acquired.complete = true;
        return owner;
    } catch (...) {
        if (acquired.failure == NativeSceneResourceAcquired::Failure::none)
            acquired.failure = NativeSceneResourceAcquired::Failure::native_call;
        if (state == 3) {
            try { context.registry_storage.actual_00bf65ac(
                reinterpret_cast<void*>(frame.name_or_ambient_argument)); }
            catch (...) { std::terminate(); }
        }
        unwind_prefix(owner, state, context);
        throw;
    }
}

void destroy_native_scene_resource_00b82ed0(void* owner,
    NativeSceneResourceStorageContext& context) {
    int state = 2;
    write(owner, 0, 0x00d63168u);
    try {
        void* const ambient = pointer(owner, 0x10);
        if (ambient) {
            const long count = context.ambient.decrement_00ce2220(
                static_cast<volatile long*>(at(ambient, 4)));
            terminal_if_zero(ambient, count, context);
            write(owner, 0x10, 0); // clear after callback even if it changed this cell
        }
        void* const begin = pointer(owner, 0x28);
        if (begin) context.registry_storage.actual_00bf65ac(begin);
        write(owner, 0x28, 0);
        write(owner, 0x2c, 0);
        write(owner, 0x30, 0);
        destroy_native_scene_registry_list_00b829d0(at(owner, 0x18), context.registry_storage);
        void* const name = pointer(owner, 0x0c);
        state = 0; // B82F45: no name-cleanup retry if getter fails
        if (name) {
            const Word size = read(owner, 8) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                context.strings.actual_published_01090aa8,
                context.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, name, size,
                context.strings.actual_small_returns_disabled_01090aa4);
        }
        state = -1;
        destroy_native_ref_counted_base_00bd30f0(owner);
    } catch (...) { unwind_prefix(owner, state, context); throw; }
}

void* delete_native_scene_resource_00b83410(void* owner,
    const volatile Word& flags, NativeSceneResourceStorageContext& context) {
    destroy_native_scene_resource_00b82ed0(owner, context);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0)
        context.registry_storage.actual_00bf65ac(owner);
    return owner;
}

NativeSceneResourceReference::NativeSceneResourceReference(void* identity,
    NativeSceneResourceIdentityContext& context)
    : RenderCommandReference(bind_count(identity, context)), identity_(identity), context_(context) {
    context_.storage.ambient.owners.bind(identity_, *this);
}
NativeSceneResourceReference::~NativeSceneResourceReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeSceneResourceReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeSceneResourceReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.storage.ambient.owners.unbind(identity_, *this);
}
void* NativeSceneResourceReference::delete_scalar_00b83410(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("scene resource deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_scene_resource_00b83410(identity_, flags, context_.storage); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeSceneResourceReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0u);
        ResourceDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
