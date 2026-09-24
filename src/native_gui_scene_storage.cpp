#include "bsp/native_gui_scene_storage.hpp"
#include "bsp/native_node_tree_retirement.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_ref_counted.hpp"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
// Actual bytes only: no owning string, host root list, or resource companion.
// Starting this lifetime must not change the preimage seen by the weak base.
struct ActualSceneStorage {
    Word profile;
    std::atomic<std::int32_t> count;
    void* weak;
    void* root;
    Word name_length;
    void* name_data;
    Word scalar;
    void* lighting;
    Word field_20;
};
static_assert(sizeof(ActualSceneStorage) == 0x24);
static_assert(offsetof(ActualSceneStorage, count) == 4);
static_assert(offsetof(ActualSceneStorage, weak) == 8);
static_assert(std::is_standard_layout_v<ActualSceneStorage>);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(std::is_trivially_destructible_v<ActualSceneStorage>);
void establish_storage(void* owner) {
    if (!owner || reinterpret_cast<Word>(owner) % alignof(ActualSceneStorage))
        throw std::invalid_argument("GUI scene constructor requires aligned actual24h storage");
    std::byte preimage[sizeof(ActualSceneStorage)];
    std::memcpy(preimage, owner, sizeof(preimage));
    auto* actual = ::new (owner) ActualSceneStorage;
    std::int32_t count_preimage;
    std::memcpy(&count_preimage, preimage + 4, 4);
    actual->count.store(count_preimage, std::memory_order_relaxed);
    std::memcpy(owner, preimage, 4);
    std::memcpy(static_cast<std::byte*>(owner) + 8, preimage + 8, sizeof(preimage) - 8);
}
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void* pointer(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(read(p, offset));
}
NativeWeakOwnerView weak_view(void* owner) noexcept {
    return {owner, *static_cast<Word*>(owner),
        *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4))),
        *static_cast<void**>(at(owner, 8))};
}
__declspec(noinline) void store_live_one(void* owner, const volatile Word& one) noexcept {
    __asm {
        mov eax, owner
        mov edx, one
        movss xmm0, dword ptr [edx]
        movss dword ptr [eax+18h], xmm0
    }
}
void require_slot(Word profile, NativeGuiSceneIdentityContext& c, unsigned slot, Word target) {
    if (profile != 0x00d62d48u || !c.scene_profile_00d62d48 ||
        c.scene_profile_00d62d48[slot] != target)
        throw std::logic_error("actual GUI scene lacks its current native slot binding");
}
std::atomic<std::int32_t>& bind_count(void* owner, NativeGuiSceneIdentityContext& c,
    NativeGuiSceneDestroyFrame& frame, NativeGuiSceneLifetimeAcquired& acquired) {
    if (!owner || reinterpret_cast<Word>(owner) % 4 || c.storage.owners.find(owner) ||
        frame.used != 0 || acquired.started)
        throw std::invalid_argument("GUI scene binding requires unregistered live storage and fresh diagnostics");
    require_slot(read(owner), c, 0, 0x00bd30e0u);
    require_slot(read(owner), c, 1, 0x00b72580u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("GUI scene binding requires completed actual construction");
    return count;
}
class SceneDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    SceneDeleteCalls(void* owner, NativeGuiSceneReference& reference, NativeGuiSceneIdentityContext& context)
        : owner_(owner), reference_(reference), context_(context) {}
    void delete_vslot04(void* owner, Word captured_profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("GUI scene terminal requires same identity and flags1");
        require_slot(captured_profile, context_, 1, 0x00b72580u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b72580(actual_flags);
    }
private:
    void* const owner_;
    NativeGuiSceneReference& reference_;
    NativeGuiSceneIdentityContext& context_;
};
} // namespace

void* construct_native_gui_scene_storage_00b724e0(void* owner,
    NativeGuiSceneConstructFrame& frame, NativeGuiSceneStorageContext& context,
    NativeGuiSceneConstructAcquired& acquired) {
    if (acquired.started) throw std::invalid_argument("GUI scene constructor requires fresh diagnostics");
    establish_storage(owner);
    acquired.started = true;
    int state = -1;
    try {
        acquired.active_call_site = 0x00b72500;
        context.weak.construct_00925490(weak_view(owner));
        acquired.weak_constructed = true;
        void* const header = at(owner, 0x10);
        write(owner, 0, 0x00d62d48);
        write(owner, 0x0c, 0);
        state = acquired.native_eh_state = 0; // B72513
        write(header, 0, 0);
        write(header, 4, 0);
        const void* const source = reinterpret_cast<const void*>(frame.name_argument); // B7251C
        const bool self = header == source;
        state = acquired.native_eh_state = 1; // B72522
        acquired.name_initialized = true;
        write(owner, 0x1c, 0);
        write(owner, 0x20, 0);
        if (!self) {
            const Word length = read(source);
            acquired.active_call_site = 0x00b72536;
            resize_native_string_header_0041dd40(header, context.strings, length, true);
            if (read(source) != 0) {
                const Word copied = read(header); // B7253F
                const void* const data = pointer(source, 4); // B72541
                void* const destination = pointer(header, 4); // B72544
                acquired.active_call_site = 0x00b7254a;
                std::memmove(destination, data, copied);
            }
        }
        store_live_one(owner, context.actual_one_00d7a24c); // B72552/B7255F
        acquired.complete = true;
        return owner;
    } catch (...) {
        try {
            while (state >= 0) {
                const int current = state--;
                acquired.native_eh_state = state;
                if (current == 1) {
                    destroy_native_string_header_0041dd20(at(owner, 0x10), context.strings);
                    acquired.name_cleanup_completed = true;
                } else {
                    context.weak.destroy_00925540(weak_view(owner));
                    acquired.weak_cleanup_completed = true;
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}

void destroy_native_gui_scene_storage_00b72430(void* owner,
    NativeGuiSceneDestroyFrame& frame, NativeGuiSceneStorageContext& context,
    NativeGuiSceneLifetimeAcquired& acquired) {
    if (acquired.started || frame.used != 0)
        throw std::invalid_argument("GUI scene destruction requires fresh diagnostics");
    acquired.started = true;
    int state = -1;
    try {
        write(owner, 0, 0x00d62d48);
        void* const lighting = pointer(owner, 0x1c);
        acquired.captured_lighting = lighting;
        state = acquired.native_eh_state = 1; // B72459
        if (lighting) {
            acquired.active_call_site = 0x00b72467;
            const long count = context.decrement_00ce2220(static_cast<volatile long*>(at(lighting, 4)));
            acquired.decrement_completed = true;
            if (count == 0) {
                acquired.active_call_site = 0x00b72477;
                auto& reference = context.owners.resolve_actual(lighting);
                if (static_cast<void*>(&reference.reference_count) != at(lighting, 4))
                    throw std::logic_error("GUI scene lighting terminal requires SAME actual count");
                reference.release_zero_references();
                acquired.terminal_completed = true;
            }
            write(owner, 0x1c, 0); // B72479, AFTER terminal even if replaced
            acquired.lighting_cleared = true;
        }
        while (pointer(owner, 0x0c)) { // B72480/B7248E
            void* const root = pointer(owner, 0x0c); // B72486
            acquired.last_root_identity = root;
            acquired.active_call_site = 0x00b72489;
            if (!context.tree || &context.tree->owners != static_cast<NativeRenderActualOwners*>(&context.owners))
                throw std::logic_error("GUI scene roots require genuine tree retirement in SAME owner registry");
            if (!frame.root_calls || frame.used >= frame.capacity || frame.root_calls[frame.used].started)
                throw std::length_error("GUI scene prepared root acquisition capacity exhausted");
            auto& root_acquired = frame.root_calls[frame.used++];
            unlink_native_node_tree_00b6dfa0(root, *context.tree, root_acquired);
            ++acquired.completed_root_calls; // metadata only; root may be dead
        }
        void* const name = pointer(owner, 0x14); // B72494
        acquired.captured_name = name;
        state = acquired.native_eh_state = 0; // B72499, BEFORE getter/return
        if (name) {
            const Word size = read(owner, 0x10) + 1u; // B724A0/A5
            acquired.captured_name_size = size;
            acquired.active_call_site = 0x00b724aa;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                context.strings.actual_published_01090aa8,
                context.strings.actual_manager_publication_01090aa0);
            acquired.active_call_site = 0x00b724b1;
            return_native_string_pool_00bd1510(pool, name, size,
                context.strings.actual_small_returns_disabled_01090aa4);
            acquired.name_return_completed = true;
        }
        state = acquired.native_eh_state = -1; // B724B8
        acquired.active_call_site = 0x00b724c0;
        context.weak.destroy_00925540(weak_view(owner));
        acquired.weak_destroyed = true;
        acquired.complete = true;
    } catch (...) {
        try {
            while (state >= 0) {
                const int current = state--;
                acquired.native_eh_state = state;
                if (current == 1) {
                    destroy_native_string_header_0041dd20(at(owner, 0x10), context.strings);
                    acquired.name_unwind_completed = true;
                } else {
                    context.weak.destroy_00925540(weak_view(owner));
                    acquired.weak_unwind_completed = true;
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}

void* delete_native_gui_scene_storage_00b72580(void* owner,
    const volatile Word& flags, NativeGuiSceneDestroyFrame& frame,
    NativeGuiSceneStorageContext& context, NativeGuiSceneLifetimeAcquired& acquired) {
    destroy_native_gui_scene_storage_00b72430(owner, frame, context, acquired);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
        acquired.active_call_site = 0x00b72590;
        singleton_lifetime_free(owner);
        acquired.freed = true; // metadata only; no owner read after free
    }
    return owner;
}

NativeGuiSceneReference::NativeGuiSceneReference(void* identity,
    NativeGuiSceneIdentityContext& context, NativeGuiSceneDestroyFrame& frame,
    NativeGuiSceneLifetimeAcquired& acquired)
    : RenderCommandReference(bind_count(identity, context, frame, acquired)),
      identity_(identity), context_(context), frame_(frame), acquired_(acquired) {
    context_.storage.owners.bind(identity_, *this);
}
NativeGuiSceneReference::~NativeGuiSceneReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGuiSceneReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGuiSceneReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.storage.owners.unbind(identity_, *this);
}
void* NativeGuiSceneReference::delete_scalar_00b72580(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("GUI scene scalar deletion requires its live canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_gui_scene_storage_00b72580(identity_, flags, frame_, context_.storage, acquired_); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeGuiSceneReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0u);
        SceneDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
