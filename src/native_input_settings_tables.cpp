#include "bsp/native_input_settings_tables.hpp"
#include "bsp/native_input_configuration_modifiers.hpp"
#include "bsp/native_input_deadline_map_lookup_adapter.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(void* p, Word o) noexcept { return pointer(address(p) + o); }
template<class T = Word> T read(void* p, Word o = 0) noexcept {
    T result; std::memcpy(&result, at(p, o), sizeof result); return result;
}
template<class T = Word> void write(void* p, Word o, T value) noexcept {
    std::memcpy(at(p, o), &value, sizeof value);
}
Word distance(Word a, Word b, unsigned shift) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(b - a) >> shift);
}
enum Object : unsigned { Keyboard, Device, NameValue, Section, Probe, Element,
    Scratch, Number, OuterKey, OuterValue, NamesKey, NamesValue, Names,
    Conflicts, Groups, Controllers, Globals, ObjectCount };
enum String : unsigned { DeviceName, InputName, SensitivityName, BareName,
    FirstAxisName, SharedName, Path, StringCount };

// One bounded cleanup stack preserves interleaved Lua/string lifetimes,
// including globals destroyed while their returned child remains tracked.
// Native private object padding and stack-header aliases are not exposed.
struct Frame {
    struct Action { bool string; unsigned index; } actions[32];
    NativeLuaObjectStorage objects[ObjectCount];
    NativeString strings[StringCount];
    unsigned count = 0;
    NativeStringStorage& storage;
    explicit Frame(NativeStringStorage& s) : storage(s) {}
    ~Frame() noexcept { while (count) release(actions[count - 1].string, actions[count - 1].index); }
    void arm(bool string, unsigned index) noexcept { actions[count++] = {string,index}; }
    void release(bool string, unsigned index) {
        unsigned i = count;
        while (i && (actions[i - 1].string != string || actions[i - 1].index != index)) --i;
        if (!i) return;
        for (unsigned j = i; j < count; ++j) actions[j - 1] = actions[j];
        --count; // normal FH3 sites disarm before their cleanup call
        if (string) destroy_native_string_header_0041dd20(&strings[index], storage);
        else destroy_native_lua_object_00b67700(objects[index]);
    }
    void close(Object slot) { release(false,slot); }
    void close(String slot) { release(true,slot); }
    NativeLuaObjectStorage& obj(Object slot) noexcept { return objects[slot]; }
    void globals(Object out, NativeLuaStateStorage& owner) {
        native_lua_globals_00b67980(owner,&obj(out)); arm(false,out);
    }
    void named(Object out, Object parent, const char* key) {
        native_lua_get_by_name_00b67800(obj(parent),&obj(out),key); arm(false,out);
    }
    void index(Object out, Object parent, Word key) {
        native_lua_get_by_index_00b67720(obj(parent),&obj(out),static_cast<std::int32_t>(key)); arm(false,out);
    }
    void empty(Object slot) { construct_native_lua_object_00b65f50(&obj(slot)); arm(false,slot); }
    void assign(Object destination, Object source) { assign_native_lua_object_00b67690(obj(destination),obj(source)); }
    bool present(Object parent, Word key) {
        index(Probe,parent,key); const bool yes = !native_lua_is_nil_00b65fb0(obj(Probe)); close(Probe); return yes;
    }
    void section(const char* key) { named(Scratch,Device,key); assign(Section,Scratch); close(Scratch); }
    void make(String slot, const char* text) { strings[slot].assign_0041e870(storage,text); arm(true,slot); }
    NativeString* str(String slot) noexcept { return &strings[slot]; }
};
std::int32_t integer(Frame& f, Object slot, NativeInputSettingsTableServices& services) {
    return native_lua_integer_00b66290(f.obj(slot),services.crt_sse2_conversion);
}
void first(Frame& f, Object table, Object key, Object value) {
    native_lua_iterate_first_00b67080(f.obj(table),f.obj(key),f.obj(value));
}
void next(Frame& f, Object table, Object key, Object value) {
    native_lua_iterate_next_00b67190(f.obj(table),f.obj(key),f.obj(value));
}
bool ended(Frame& f, Object key) { return native_lua_is_unbound_00b66420(f.obj(key)); }
void* last_row(void* header) {
    const auto end = read(header,8);
    if (read(header,4) > end) _invalid_parameter_noinfo();
    const auto row = end - 0x10u;
    if (row > read(header,8) || row < read(header,4)) _invalid_parameter_noinfo();
    if (row >= read(header,8)) _invalid_parameter_noinfo();
    return pointer(row);
}
void* checked_element(void* header, Word index, unsigned shift) {
    const auto begin = read(header,4);
    if (!begin || index >= distance(begin,read(header,8),shift)) _invalid_parameter_noinfo();
    return pointer(read(header,4) + (index << shift));
}
void overwrite_string(void* header, const char* text, NativeStringStorage& storage) {
    const auto length = text ? static_cast<Word>(std::strlen(text)) : 0u;
    resize_native_string_header_0041dd40(header,storage,length,false);
    void* const destination = read<void*>(header,4);
    if (destination) {
        const auto current_length = read(header);
        if (current_length) std::memcpy(destination,text,current_length);
    }
}
void write_live_one(void* destination, const volatile float& value) noexcept {
    const volatile float* source = &value;
    __asm {
        mov eax, source
        mov edx, destination
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx], xmm0
    }
}
void store_base_scale(void* destination, float value, const volatile float& epsilon) noexcept {
    const volatile float* replacement = &epsilon;
    __asm {
        mov edx, destination
        fld dword ptr value
        fst dword ptr [edx]
        fldz
        fxch st(1)
        fucomip st, st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp unchanged
        mov eax, replacement
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx], xmm0
    unchanged:
    }
}
void codes(Frame& f, Object table, Object value, void* header,
    NativeInputSettingsTableServices& services) {
    for (Word index = 1;; ++index) {
        f.index(value,table,index);
        if (native_lua_is_nil_00b65fb0(f.obj(value))) { f.close(value); break; }
        auto code = integer(f,value,services);
        append_input_checked_word_storage(header,&code);
        f.close(value);
    }
}

void parse_tables(void* settings, NativeLuaStateStorage& lua, NativeInputSettingsTableServices& s) {
    Frame f(s.scripts.strings); auto& c = s.containers; auto& trees = s.trees;
    void* tree = at(settings,8);
    c.call_006a7540(tree,pointer(read(pointer(read(tree,4)),4)));
    void* head = read<void*>(tree,4); write(head,4,address(head)); write(tree,8,0u);
    head = read<void*>(tree,4); write(head,0,address(head)); head = read<void*>(tree,4); write(head,8,address(head));
    void* order = at(settings,0x14); const auto end = read(order,8);
    if (read(order,4) > end) _invalid_parameter_noinfo();
    const auto begin = read(order,4);
    if (begin > read(order,8)) _invalid_parameter_noinfo();
    NativeKeyboardTreeIterator erased;
    c.call_004954f0(order,&erased,{order,pointer(begin)},{order,pointer(end)});
    f.globals(Globals,lua); f.named(Keyboard,Globals,"KeyboardSetup"); f.close(Globals);
    for (Word device_index = 1; f.present(Keyboard,device_index); ++device_index) {
        f.index(Device,Keyboard,device_index); f.named(NameValue,Device,"Name");
        f.make(DeviceName,native_lua_string_00b662b0(f.obj(NameValue))); f.close(NameValue);
        void* device = trees.device_0055c110(at(settings,8),f.str(DeviceName));
        c.call_00450540(at(settings,0x14),f.str(DeviceName));
        f.named(Section,Device,"Inputs");
        for (Word i = 1; f.present(Section,i); ++i) {
            f.index(Element,Section,i);
            if (native_lua_is_string_00b660a0(f.obj(Element))) {
                f.make(BareName,native_lua_string_00b662b0(f.obj(Element)));
                c.call_00450540(at(device,0x30),f.str(BareName)); f.close(BareName); f.close(Element); continue;
            }
            f.index(Scratch,Element,1); f.make(InputName,native_lua_string_00b662b0(f.obj(Scratch))); f.close(Scratch);
            void* input = trees.input_codes_006a44b0(device,f.str(InputName));
            c.call_00450540(at(device,0x30),f.str(InputName));
            void* bindings = trees.bindings_006a45c0(at(device,0xc),f.str(InputName));
            c.call_006a0db0(bindings,2,{0xffffffffu,0,0,0,s.descriptor_flag_stack_preimage & 0xffffff00u});
            void* reverse = trees.reverse_006a4ca0(at(device,0x6c),f.str(InputName)); c.call_0049df50(reverse,2,0);
            f.index(Probe,Element,2); codes(f,Probe,Number,input,s); f.close(Probe); f.close(InputName); f.close(Element);
        }
        f.section("Sensitivities");
        for (Word i = 1; f.present(Section,i); ++i) {
            f.index(Number,Section,i); f.index(Scratch,Number,1);
            f.make(SensitivityName,native_lua_string_00b662b0(f.obj(Scratch))); f.close(Scratch);
            void* sensitivity = c.call_0055a9a0(at(device,0x18),f.str(SensitivityName));
            c.call_00450540(at(device,0x40),f.str(SensitivityName));
            f.index(Element,Number,2); write(sensitivity,0,static_cast<Word>(integer(f,Element,s))); f.close(Element);
            f.index(Probe,Number,3); codes(f,Probe,Element,at(sensitivity,4),s);
            void* multiplier = trees.multiplier_00444be0(at(device,0x24),f.str(SensitivityName));
            write_live_one(multiplier,s.one_00d7a24c); f.close(Probe); f.close(SensitivityName); f.close(Number);
        }
        f.section("BaseSensitivities"); f.empty(OuterKey); f.empty(OuterValue); first(f,Section,OuterKey,OuterValue);
        while (!ended(f,OuterKey)) {
            auto key = integer(f,OuterKey,s); void* classes = trees.code_classes_006a5aa0(at(device,0x50),&key);
            f.empty(Element); f.empty(Number); first(f,OuterValue,Element,Number);
            while (!ended(f,Element)) {
                auto kind = integer(f,Element,s); float* value = subscript_input_deadline_map_storage(classes,&kind);
                const float number = native_lua_number_00b66270(f.obj(Number)); store_base_scale(value,number,s.base_zero_replacement_00cf7fe8);
                next(f,OuterValue,Element,Number);
            }
            f.close(Number); f.close(Element); next(f,Section,OuterKey,OuterValue);
        }
        f.section("AxisPairs");
        for (Word i = 1; f.present(Section,i); ++i) {
            f.index(Probe,Section,i); void* pairs = at(device,0x5c);
            c.call_006a6350(pairs,i,{s.vector_opaque_stack_preimage,0,0,0}); void* row = last_row(pairs);
            f.index(Scratch,Probe,1); f.make(FirstAxisName,native_lua_string_00b662b0(f.obj(Scratch)));
            c.call_00450540(row,f.str(FirstAxisName)); f.close(FirstAxisName); f.close(Scratch);
            f.index(Scratch,Probe,2); f.make(SharedName,native_lua_string_00b662b0(f.obj(Scratch)));
            c.call_00450540(row,f.str(SharedName)); f.close(SharedName); f.close(Scratch); f.close(Probe);
        }
        f.section("Min1SensHacks");
        if (!native_lua_is_nil_00b65fb0(f.obj(Section))) {
            for (Word i = 1; f.present(Section,i); ++i) {
                f.index(Scratch,Section,i); auto key = integer(f,Scratch,s); std::array<Word,3> inserted;
                c.call_0069fa40(at(device,0x78),inserted.data(),&key); f.close(Scratch);
            }
        }
        f.close(OuterValue); f.close(OuterKey); f.close(Section); f.close(DeviceName); f.close(Device);
    }
    f.globals(Scratch,lua); f.named(Names,Scratch,"InputNames"); f.close(Scratch);
    f.empty(NamesKey); f.empty(NamesValue); first(f,Names,NamesKey,NamesValue);
    while (!ended(f,NamesKey)) {
        f.make(SharedName,native_lua_string_00b662b0(f.obj(NamesKey)));
        void* record = c.call_006a1e70(at(settings,0x24),f.str(SharedName)); f.close(SharedName);
        f.index(NameValue,NamesValue,1); write(record,0,static_cast<Word>(integer(f,NameValue,s))); f.close(NameValue);
        f.index(NameValue,NamesValue,2); write(record,0xc,static_cast<Word>(integer(f,NameValue,s))); f.close(NameValue);
        next(f,Names,NamesKey,NamesValue);
    }
    f.globals(NameValue,lua); f.named(Conflicts,NameValue,"Conflicts"); f.close(NameValue);
    f.named(Groups,Conflicts,"Groups");
    for (Word group = 1; f.present(Groups,group); ++group) {
        f.index(OuterKey,Groups,group); void* outer = at(settings,0x30);
        c.call_006a79a0(outer,group,{s.vector_opaque_stack_preimage,0,0,0}); void* rows = last_row(outer);
        for (Word pair = 1; f.present(OuterKey,pair); ++pair) {
            f.index(Probe,OuterKey,pair); c.call_006a6350(rows,pair,{s.vector_opaque_stack_preimage,0,0,0});
            void* row = last_row(rows); c.call_0049e050(row,2,{0,0});
            for (Word index = 0; index < 2; ++index) {
                f.index(Element,Probe,index + 1); void* string = checked_element(row,index,3);
                const char* text = native_lua_string_00b662b0(f.obj(Element)); overwrite_string(string,text,s.scripts.strings); f.close(Element);
            }
            f.close(Probe);
        }
        f.close(OuterKey);
    }
    f.named(OuterKey,Conflicts,"Pairs"); f.assign(Groups,OuterKey); f.close(OuterKey);
    for (Word pair = 1; f.present(Groups,pair); ++pair) {
        f.index(Probe,Groups,pair); void* outer = at(settings,0x40);
        c.call_006a4710(outer,pair,{s.vector_opaque_stack_preimage,0,0,0}); void* row = last_row(outer); c.call_00492210(row,2,0);
        for (Word index = 0; index < 2; ++index) {
            f.index(OuterKey,Probe,index + 1); void* value = checked_element(row,index,2);
            write(value,0,static_cast<Word>(integer(f,OuterKey,s)) - 1u); f.close(OuterKey);
        }
        f.close(Probe);
    }
    f.globals(OuterKey,lua); f.named(Probe,OuterKey,"DEVINPUTS"); f.assign(Conflicts,Probe); f.close(Probe); f.close(OuterKey);
    std::uint8_t enabled = 0;
    if (native_lua_is_boolean_00b66000(f.obj(Conflicts))) enabled = native_lua_boolean_00b66250(f.obj(Conflicts)) ? 1u : 0u;
    write<std::uint8_t>(settings,0x50,enabled);
    f.str(Path)->resize_0041dd40(s.scripts.strings,0x2b,true);
    if (f.str(Path)->data()) std::memcpy(f.str(Path)->data(),"Scripts\\datatables\\ControllerInputNames.lua",f.str(Path)->length() + 1u);
    f.arm(true,Path); run_native_lua_file_00b69d40(lua,*f.str(Path),0,s.scripts.strings,s.scripts.files); f.close(Path);
    f.globals(Probe,lua); f.named(Controllers,Probe,"ControllerInputNames"); f.close(Probe);
    first(f,Controllers,NamesKey,NamesValue);
    while (!ended(f,NamesKey)) {
        f.make(SharedName,native_lua_string_00b662b0(f.obj(NamesKey)));
        void* controller = c.call_006a6900(at(settings,0x60),f.str(SharedName)); f.close(SharedName);
        f.empty(OuterKey); f.empty(Element); first(f,NamesValue,OuterKey,Element);
        while (!ended(f,OuterKey)) {
            auto code = integer(f,OuterKey,s); const char* text = native_lua_string_00b662b0(f.obj(Element));
            void* name = c.call_006a1f80(controller,&code); overwrite_string(name,text,s.scripts.strings);
            next(f,NamesValue,OuterKey,Element);
        }
        f.close(Element); f.close(OuterKey); next(f,Controllers,NamesKey,NamesValue);
    }
    f.close(Controllers); f.close(Groups); f.close(Conflicts); f.close(NamesValue); f.close(NamesKey); f.close(Names); f.close(Keyboard);
}
} // namespace
void load_native_input_settings_tables_006a7be0(void* settings, NativeInputSettingsTableServices& services) {
    NativeInputSettingsScriptFrame frame;
    if (load_native_input_settings_scripts_prefix_006a7be0(settings,services.scripts,frame) == NativeInputSettingsScriptPrefixResult::guard_return) return;
    parse_tables(settings,frame.temporary_lua(),services);
    frame.close();
}
} // namespace bsp
