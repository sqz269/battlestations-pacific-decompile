#include "bsp/native_input_settings_lifetime.hpp"
#include "bsp/global_config.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word o) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + o);
}
template<class T = Word> T read(void* p, Word o = 0) noexcept {
    T result; std::memcpy(&result, at(p,o), sizeof result); return result;
}
template<class T = Word> void write(void* p, Word o, T value) noexcept {
    std::memcpy(at(p,o), &value, sizeof value);
}

// Consumed full-member cleanup contracts from constructor/destructor unwind.
// Tree payload and range destruction remain required library operations.
struct Members {
    void* settings;
    NativeInputSettingsLifetimeContext& context;
    int state = 9;
    bool armed = true;
    ~Members() noexcept {
        if (!armed) return;
        while (state > 0) {
            const int member = state--;
            destroy_member(member);
        }
        context.publication_00e198e8 = nullptr;
        write(settings,0,0x00ce3818u);
    }
    void tree(Word offset) {
        void* header = at(settings,offset);
        void* const head = read<void*>(header,4);
        void* const first_node = read<void*>(head);
        NativeKeyboardTreeIterator output;
        const NativeKeyboardTreeIterator first{header,first_node}, last{header,head};
        auto& c = context.containers;
        switch (offset) {
        case 8: case 0x6c: c.call_006a7aa0(header,&output,first,last); break;
        case 0x60: c.call_006a6a20(header,&output,first,last); break;
        case 0x54: c.call_0069fe70(header,&output,first,last); break;
        case 0x24: c.call_006a1aa0(header,&output,first,last); break;
        }
        singleton_lifetime_free(read<void*>(header,4));
        write(header,4,0u); write(header,8,0u);
    }
    void vector(Word offset, void* /*native_opaque_stack_word*/) {
        void* header = at(settings,offset);
        void* const begin = read<void*>(header,4);
        if (begin) {
            void* const end = read<void*>(header,8);
            switch (offset) {
            case 0x40: destroy_native_input_settings_conflict_pair_range_0069eea0(begin,end); break;
            case 0x30: destroy_native_input_settings_group_range_006a6ee0(begin,end,context.tables.scripts.strings); break;
            case 0x14: destroy_global_config_name_range_00432050(begin,end,context.tables.scripts.strings); break;
            }
            singleton_lifetime_free(read<void*>(header,4));
        }
        write(header,4,0u); write(header,8,0u); write(header,12,0u);
    }
    void close_lua() {
        close_native_lua_state_00b669a0(*static_cast<NativeLuaStateStorage*>(at(settings,0x78)));
    }
    void destroy_member(int member) {
        switch (member) {
        case 9: close_lua(); break;
        case 8: tree(0x6c); break;
        case 7: tree(0x60); break;
        case 6: tree(0x54); break;
        case 5: vector(0x40,at(settings,0x40)); break;
        case 4: vector(0x30,at(settings,0x30)); break;
        case 3: tree(0x24); break;
        case 2: vector(0x14,at(settings,0x14)); break;
        case 1: tree(8); break;
        }
    }
};
} // namespace

void* construct_native_input_settings_006ab6b0(
    void* settings, NativeInputSettingsLifetimeContext& context) {
    // Prefix failure already unwinds its constructed empty members. Arm the
    // nonempty cleanup only after every header and persistent owner exists.
    construct_native_input_settings_prefix_006ab6b0(settings,context.publication_00e198e8);
    Members members{settings,context};
    load_native_input_settings_tables_006a7be0(settings,context.tables);
    members.armed = false;
    return settings;
}

void* get_native_input_settings_005547d0(NativeInputSettingsLifetimeContext& context) {
    void* const initial = context.publication_00e198e8;
    if (initial) return initial;
    SoundLifetimeAccess lifetime(context.manager_publication_01090aa0);
    {
        CapturedSoundLifetimeSection section(lifetime);
        if (!context.publication_00e198e8) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object,0x540,0x540});
            void* result;
            try {
                result = allocation ? construct_native_input_settings_006ab6b0(allocation,context) : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            context.publication_00e198e8 = result;
            auto current_manager = lifetime.get_manager_00415350();
            current_manager->register_object(context.publication_00e198e8);
        }
    }
    return context.publication_00e198e8;
}

void destroy_native_input_settings_006aa460(
    void* settings, NativeInputSettingsLifetimeContext& context) {
    write(settings,0,0x00cf81ccu);
    Members members{settings,context};
    // B65E80 and B669A0 have the same actual close operation, but are two
    // distinct native calls separated by a state transition.
    members.close_lua();
    members.state = 8; members.close_lua();
    members.state = 7; members.tree(0x6c);
    members.state = 6; members.tree(0x60);
    members.state = 5; members.tree(0x54);
    // The original keeps state5 here: a throwing range call would unwind
    // through the complete pair-vector member before proceeding to groups.
    members.vector(0x40,settings);
    members.state = 3; members.vector(0x30,settings);
    members.state = 2; members.tree(0x24);
    members.state = 1; members.vector(0x14,settings);
    members.state = 0; members.tree(8);
    // Members performs the unconditional publication clear and base stamp.
}

void* scalar_delete_native_input_settings_006ab800(
    void* settings, std::uint32_t flags, NativeInputSettingsLifetimeContext& context) {
    destroy_native_input_settings_006aa460(settings,context);
    if ((flags & 1u) != 0) singleton_lifetime_free(settings);
    return settings;
}
} // namespace bsp
