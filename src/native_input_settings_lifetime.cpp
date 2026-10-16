#include "bsp/native_input_settings_lifetime.hpp"
#include "bsp/native_input_device_tree_range.hpp"
#include "bsp/native_input_controller_tree_range.hpp"
#include "bsp/native_input_default_scalar_tree_range.hpp"
#include "bsp/native_input_conflict_vector_tidy.hpp"
#include "bsp/native_string_vector_group_cleanup.hpp"
#include "bsp/native_input_preset_tree_range.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_input_device_map_cleanup.hpp"
#include "bsp/native_input_controller_tree_destroy_instances.hpp"
#include "bsp/native_input_default_scalar_tree_destroy.hpp"
#include "bsp/native_input_preset_tree_destroy.hpp"
#include "bsp/native_string_vector_group_tidies.hpp"
#include "bsp/global_config.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"
#include "bsp/native_input_settings_tree_cleanup.hpp"
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
// Tree payload, range and vector destruction use concrete source storage.
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
        auto& strings = context.tables.scripts.strings;
        switch (offset) {
        case 8: case 0x6c: erase_native_input_device_tree_range_006a7aa0(header,&output,first,last,strings); break;
        case 0x60: erase_native_input_controller_tree_range_006a6a20(header,&output,first,last,strings); break;
        case 0x54: erase_native_input_default_tree_range_0069fe70(header,&output,first,last,strings); break;
        case 0x24: erase_native_input_preset_tree_range_006a1aa0(header,&output,first,last,strings); break;
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
// Only the fixed normal operation order is shared. Legacy field access, guard
// policy and overload selection remain in Members above.
template<class Operations>
void destroy_in_normal_order(Operations& operations) {
    operations.first_lua();
    operations.second_lua();
    operations.device_6c();
    operations.controller_60();
    operations.scalar_54();
    operations.conflict_40();
    operations.group_30();
    operations.preset_24();
    operations.names_14();
    operations.device_08();
}

struct LegacyNormalOps {
    Members& members;
    void first_lua() { members.close_lua(); }
    void second_lua() { members.state=8; members.close_lua(); }
    void device_6c() { members.state=7; members.tree(0x6c); }
    void controller_60() { members.state=6; members.tree(0x60); }
    void scalar_54() { members.state=5; members.tree(0x54); }
    void conflict_40() { members.vector(0x40,members.settings); }
    void group_30() { members.state=3; members.vector(0x30,members.settings); }
    void preset_24() { members.state=2; members.tree(0x24); }
    void names_14() { members.state=1; members.vector(0x14,members.settings); }
    void device_08() { members.state=0; members.tree(8); }
};

Word raw_read(void* object, Word offset) noexcept {
    return *static_cast<const volatile Word*>(at(object,offset));
}
void raw_write(void* object, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(object,offset))=value;
}
void* raw_pointer(void* object, Word offset) noexcept {
    return reinterpret_cast<void*>(raw_read(object,offset));
}

// Logical DACEA0 cleanup states, not the original EBP/FS/FH3 frame. No
// per-child noexcept bridge: a secondary C++ cleanup exception terminates at
// this guard's existing-language noexcept boundary.
struct RawMembers {
    void* settings;
    NativeStringRawPoolContext& strings;
    void* volatile& publication;
    int state=-1;

    ~RawMembers() noexcept {
        while (state>0) {
            const int member=state--;
            destroy_member(member);
        }
        if (state==0) {
            // Complete 69BA20 effects. Its old uint32& interface cannot bind
            // this volatile publication cell. Keep state0 through both stores.
            publication=nullptr;
            raw_write(settings,0,0x00ce3818u);
        }
    }
    void close_lua() {
        // B65E80 and B669A0 have the same complete actual-storage effects.
        close_native_lua_state_00b669a0(
            *static_cast<NativeLuaStateStorage*>(at(settings,0x78)));
    }
    void destroy_member(int member) {
        switch (member) {
        case 9: close_lua(); break;
        case 8: destroy_native_input_device_map_006aa420(at(settings,0x6c),0,strings); break;
        case 7: destroy_native_input_controller_tree_006a7040(at(settings,0x60),0,strings); break;
        case 6: destroy_native_input_default_scalar_tree_006a1df0(at(settings,0x54),0); break;
        case 5: tidy_native_input_conflict_vector_006a1420(at(settings,0x40),0); break;
        case 4: tidy_native_string_vector_group_vector_006a7960(at(settings,0x30),strings); break;
        case 3: destroy_native_input_preset_tree_006a2b00(at(settings,0x24),0,strings); break;
        case 2: destroy_native_string_header_vector_004324a0(at(settings,0x14),strings); break;
        case 1: destroy_native_input_device_map_006aa420(at(settings,8),0,strings); break;
        }
    }
};

struct RawNormalOps {
    RawMembers& members;

    void first_lua() { members.state=9; members.close_lua(); }
    void second_lua() { members.state=8; members.close_lua(); }

    void finish_tree(void* tree) {
        singleton_lifetime_free(raw_pointer(tree,4));
        raw_write(tree,4,0);
        raw_write(tree,8,0);
    }
    void device(Word offset, int next_state) {
        void* const tree=at(members.settings,offset);
        void* const head=raw_pointer(tree,4);
        void* const left=raw_pointer(head,0);
        Word output[2];
        members.state=next_state; // Native head/left captures precede this.
        erase_native_raw_input_device_tree_range_006a7aa0(
            tree,0,output,tree,left,tree,head,members.strings);
        finish_tree(tree);
    }
    void device_6c() { device(0x6c,7); }
    void device_08() { device(8,0); }

    void controller_60() {
        void* const tree=at(members.settings,0x60);
        void* const head=raw_pointer(tree,4);
        void* const left=raw_pointer(head,0);
        NativeInputControllerTreeIterator output;
        const NativeInputControllerTreeIterator first{tree,left},last{tree,head};
        members.state=6;
        erase_native_input_controller_tree_range_006a6a20(
            tree,0,&output,first,last,members.strings);
        finish_tree(tree);
    }
    void scalar_54() {
        void* const tree=at(members.settings,0x54);
        void* const head=raw_pointer(tree,4);
        void* const left=raw_pointer(head,0);
        Word output[2];
        members.state=5;
        erase_native_input_default_scalar_tree_range_0069fe70(
            tree,nullptr,output,tree,left,tree,head);
        finish_tree(tree);
    }
    void preset_24() {
        void* const tree=at(members.settings,0x24);
        void* const head=raw_pointer(tree,4);
        void* const left=raw_pointer(head,0);
        NativeInputPresetTreeIterator output;
        const NativeInputPresetTreeIterator first{tree,left},last{tree,head};
        members.state=2;
        erase_native_input_preset_tree_range_006a1aa0(
            tree,0,&output,first,last,members.strings);
        finish_tree(tree);
    }
    void clear_vector(void* header) {
        raw_write(header,4,0);
        raw_write(header,8,0);
        raw_write(header,0x0c,0);
    }
    void conflict_40() {
        void* const header=at(members.settings,0x40);
        void* const first=raw_pointer(header,4);
        // State5 is deliberately unchanged: failure retries this member.
        if (first) {
            void* const last=raw_pointer(header,8);
            destroy_native_input_conflict_vector_header_range_0069eea0(
                first,last,reinterpret_cast<Word>(header),
                reinterpret_cast<Word>(members.settings));
            singleton_lifetime_free(raw_pointer(header,4));
        }
        clear_vector(header);
    }
    void group_30() {
        void* const header=at(members.settings,0x30);
        void* const first=raw_pointer(header,4);
        members.state=3;
        if (first) {
            void* const last=raw_pointer(header,8);
            destroy_native_string_vector_group_range_006a6ee0(
                first,last,reinterpret_cast<Word>(header),
                reinterpret_cast<Word>(members.settings),members.strings);
            singleton_lifetime_free(raw_pointer(header,4));
        }
        clear_vector(header);
    }
    void names_14() {
        void* const header=at(members.settings,0x14);
        void* const first=raw_pointer(header,4);
        members.state=1;
        if (first) {
            void* const last=raw_pointer(header,8);
            destroy_native_string_header_range_00432050(first,last,members.strings);
            singleton_lifetime_free(raw_pointer(header,4));
        }
        clear_vector(header);
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
    LegacyNormalOps operations{members};
    destroy_in_normal_order(operations);
    // Members performs the unconditional publication clear/base stamp.
}

void* scalar_delete_native_input_settings_006ab800(
    void* settings, std::uint32_t flags, NativeInputSettingsLifetimeContext& context) {
    destroy_native_input_settings_006aa460(settings,context);
    if ((flags & 1u) != 0) singleton_lifetime_free(settings);
    return settings;
}


// Complete480B ordinary effects with actual provided RawCtx services plus the
// ten logical cleanup actions. These new arguments/private objects do not
// preserve the original ECX/plainRET/FH3/SEH ABI or hardware-fault scopes.
void __cdecl destroy_native_raw_input_settings_006aa460(
    void* settings, NativeStringRawPoolContext& strings,
    void* volatile& publication_00e198e8) {
    RawMembers members{settings,strings,publication_00e198e8};
    raw_write(settings,0,0x00cf81ccu);
    RawNormalOps operations{members};
    destroy_in_normal_order(operations);
}
} // namespace bsp
