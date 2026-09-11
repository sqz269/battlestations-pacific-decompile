#include "bsp/lua_binding_ai.hpp"

#include <cstring>

// Reconstruction of the fourteen AI* mission Lua bindings. See
// docs/LUA_BINDING_AI.md for the per-claim evidence. These are not drop-in
// binary replacements: the native handlers are lua_CFunctions over the LuaObject
// call-frame API, and this file projects only their decoding rules and their
// native call sequences over an injected host.
namespace bsp {
namespace {

// Every handler's result count is `lua_gettop - frame.base` through 00B66400,
// never a literal. Thirteen of the fourteen push nothing, so the count is zero;
// AIGetTargetWeight pushes one number and AIGetGroupInfo builds a table.
constexpr int kNoResults = 0;

} // namespace

// ---------------------------------------------------------------------------
// Decoding rules
// ---------------------------------------------------------------------------

bool ai_enable_reads_ratios(bool enabled, int argument_count) noexcept {
    // 00A37515 tests the byte that was just stored, not the local, so a store
    // that did not take would suppress the ratios. 00A37527 then requires a
    // count of three or more.
    return enabled && argument_count >= 3;
}

float ai_clamp_defend_resource_percent(float value) noexcept {
    if (!(value >= 0.0f)) {
        // FLDZ/FCOMIP at 00A37E24 with JBE falling through to the clamp: a NaN
        // fails the ordered compare and takes the zero branch.
        return 0.0f;
    }
    if (value > kAiUnitClamp) {
        return kAiUnitClamp;
    }
    return value;
}

float ai_spawn_scene_units_weight_mul(float value) noexcept {
    // 00A37F2x stores the minimum unconditionally, then overwrites it only when
    // the argument reaches the minimum. This is deliberately not the same shape
    // as the clamp above: an argument below 0.001f yields 0.001f, not the
    // argument and not zero. The installed scripts pass 0 and so land here.
    float result = kAiWeightMulMinimum;
    if (value >= kAiWeightMulMinimum) {
        result = value;
        if (value > kAiUnitClamp) {
            result = kAiUnitClamp;
        }
    }
    return result;
}

AiTargetWeightQuery ai_target_weight_query(int argument_count,
                                           int argument_two,
                                           bool argument_three) noexcept {
    AiTargetWeightQuery query{};
    // Two independent equality tests, 00A38340 and 00A38380. With four
    // arguments the first test fails, so `mode` keeps the zero that 00A38337
    // wrote and argument 2 is never read.
    if (argument_count == 3) {
        query.mode = argument_two;
    }
    if (argument_count == 4) {
        query.flag = argument_three;
    }
    return query;
}

int ai_set_target_weight_weight_slot(int argument_count) noexcept {
    // 00A387xx compares the count with 5 and takes the short branch below it.
    // The short branch reads the weight from slot 3; the long branch reads three
    // more arguments first and takes the weight from slot 6.
    return argument_count < 5 ? 3 : 6;
}

int ai_class_index_from_name(const std::string& name,
                             const char* const* table,
                             int table_count) noexcept {
    if (table == nullptr) {
        return kAiClassNameNotFound;
    }
    for (int index = 0; index < table_count; ++index) {
        const char* entry = table[index];
        if (entry != nullptr && std::strcmp(name.c_str(), entry) == 0) {
            return index;
        }
    }
    // 00A385E9 leaves -1 in the slot when the walk runs past entry 60h.
    return kAiClassNameNotFound;
}

// ---------------------------------------------------------------------------
// Handler sequences
// ---------------------------------------------------------------------------

int lua_binding_ai_create(LuaBindingAiHost& host) {
    // 00A37310 reads no argument and pushes nothing: between the frame and the
    // result count there is exactly one CALL, at 00A373AF.
    host.ai_controller_create();
    return kNoResults;
}

int lua_binding_ai_enable(LuaBindingArgumentReader& args,
                          LuaBindingAiReader& tables,
                          LuaBindingAiHost& host) {
    const int index = args.get_integer(0);
    const bool enabled = args.get_boolean(1);
    host.store_party_enabled(index, enabled);

    if (ai_enable_reads_ratios(enabled, args.count())) {
        // Both fields fall back to 00CE3800, which is 0.5f.
        const float attack = tables.table_field_number(2, "attackRatio", kAiRatioDefault);
        const float aggressive =
            tables.table_field_number(2, "aggressiveRatio", kAiRatioDefault);
        host.store_party_ratios(index, attack, aggressive);
    }
    return kNoResults;
}

int lua_binding_ai_enable_grouping(LuaBindingArgumentReader& args,
                                   LuaBindingAiHost& host) {
    // 00A376E9 opens argument 1 first and 00A37703 argument 0 second; the group
    // is resolved from argument 0 and the boolean read from argument 1.
    void* group = host.ai_group_of_entity_argument(0);
    const bool enabled = args.get_boolean(1);
    // 00A37731 stores AL straight into group+5648h with no null test.
    if (group != nullptr) {
        auto* flag = static_cast<unsigned char*>(group) + kAiGroupGroupingEnabledOffset;
        *flag = enabled ? 1u : 0u;
    }
    return kNoResults;
}

int lua_binding_ai_merge_groups(LuaBindingAiHost& host) {
    void* into = host.ai_group_of_entity_argument(0);
    void* from = host.ai_group_of_entity_argument(1);
    host.ai_group_merge(into, from);
    // 00A37875 clears the grouping flag on the first group after the merge.
    if (into != nullptr) {
        auto* flag = static_cast<unsigned char*>(into) + kAiGroupGroupingEnabledOffset;
        *flag = 0u;
    }
    return kNoResults;
}

int lua_binding_ai_get_group_info(LuaBindingAiHost& host) {
    void* group = host.ai_group_of_entity_argument(0);
    host.new_lua_table();
    host.ai_group_write_info(group, nullptr);
    // The result count is still 00B66400's difference. 00B67930 takes a registry
    // reference, which pops the table it created, so whether this handler leaves
    // anything on the stack depends on 00A2EEE0, whose body was not read in full.
    return kNoResults;
}

int lua_binding_ai_set_command(LuaBindingAiHost& host) {
    void* group = host.ai_group_of_entity_argument(0);
    void* command = host.ai_command_from_table(group, 1);
    host.ai_group_set_command(group, command);
    return kNoResults;
}

int lua_binding_ai_set_hint_weight(LuaBindingArgumentReader& args,
                                   LuaBindingAiReader& tables,
                                   LuaBindingAiHost& host) {
    const int count = args.count();
    if (count == 2) {
        // 00A37BDE: exactly two arguments. The weight is argument 1 and the hint
        // handle is the "Ptr" field of argument 0.
        const float weight = static_cast<float>(args.get_number(1));
        void* hint = tables.lua_table_ptr_field(0);
        host.ai_hint_weight_set_global(hint, weight);
    } else if (count == 3) {
        // 00A37C4A: exactly three. The weight is argument 2, the party is the
        // integer argument 1, and the handle is again argument 0's "Ptr".
        const float weight = static_cast<float>(args.get_number(2));
        const int party = args.get_integer(1);
        void* hint = tables.lua_table_ptr_field(0);
        host.ai_hint_weight_set_for_party(hint, party, weight);
    }
    // Any other count falls through to the epilogue at 00A37CFC untouched.
    return kNoResults;
}

int lua_binding_ai_set_defend_resource_percent(LuaBindingArgumentReader& args,
                                               LuaBindingAiHost& host) {
    // 009FFC80 runs before the argument is read, at 00A37DF6.
    const int slot = host.ai_current_party_slot();
    const float percent = ai_clamp_defend_resource_percent(
        static_cast<float>(args.get_number(0)));
    host.store_defend_resource_percent(slot, percent);
    return kNoResults;
}

int lua_binding_ai_set_spawn_scene_units_weight_mul(LuaBindingArgumentReader& args,
                                                    LuaBindingAiHost& host) {
    const float value =
        ai_spawn_scene_units_weight_mul(static_cast<float>(args.get_number(0)));
    host.store_spawn_scene_units_weight_mul(value);
    return kNoResults;
}

int lua_binding_ai_set_quick_spawn_target_pos(LuaBindingArgumentReader& args,
                                              LuaBindingAiReader& tables,
                                              LuaBindingAiHost& host) {
    const int index = args.get_integer(0);
    const bool valid = args.get_boolean(1);
    float xyz[3] = {0.0f, 0.0f, 0.0f};
    if (valid) {
        // 00A38117 skips the vector entirely when the flag is false.
        tables.read_vector3(2, xyz);
    }
    // 00A38187 stores the flag unconditionally and the three floats only when it
    // is set, so a false call leaves the previous position bytes in place.
    host.store_party_quick_spawn(index, valid, xyz);
    return kNoResults;
}

int lua_binding_ai_get_target_weight(LuaBindingArgumentReader& args,
                                     LuaBindingAiHost& host) {
    void* subject = host.vehicle_class_descriptor(args.get_integer(0));
    void* target = host.vehicle_class_descriptor(args.get_integer(1));

    float weight = 0.0f;
    if (subject != nullptr && target != nullptr) {
        const int count = args.count();
        const int mode = count == 3 ? args.get_integer(2) : 0;
        const bool flag = count == 4 ? args.get_boolean(3) : false;
        const AiTargetWeightQuery query = ai_target_weight_query(count, mode, flag);
        weight = host.ai_target_weight_query(subject, target, query.mode, query.flag);
    }
    // 00A383D9 pushes the float through 00B66480 on both paths, so a failed
    // lookup still answers one result, the zero from 00A3831C.
    host.push_number_float(weight);
    return 1;
}

int lua_binding_ai_set_target_weight(LuaBindingArgumentReader& args,
                                     LuaBindingAiHost& host) {
    AiTargetWeightRule rule{};

    // Each class argument is an integer or a name: 00B660A0 decides, and the
    // string path falls back to -1 when the 97-entry table has no match.
    rule.subject_was_integer = !args.is_string(0);
    if (rule.subject_was_integer) {
        rule.subject_class = args.get_integer(0);
    }
    rule.target_was_integer = !args.is_string(1);
    if (rule.target_was_integer) {
        rule.target_class = args.get_integer(1);
    }

    rule.flag = args.get_boolean(2);
    rule.extended = args.count() >= 5;
    if (rule.extended) {
        rule.extra_a = args.get_integer(3);
        rule.extra_b = args.get_integer(4);
        rule.extra_flag = args.get_boolean(5);
    }
    rule.weight = static_cast<float>(
        args.get_number(ai_set_target_weight_weight_slot(args.count())));

    host.ai_target_weight_rule_apply(rule);
    return kNoResults;
}

int lua_binding_ai_reload_globals(LuaBindingAiHost& host) {
    host.ai_reload_globals();
    return kNoResults;
}

int lua_binding_ai_create_group(LuaBindingAiHost& host,
                                void* const* entities,
                                std::size_t entity_count) {
    // The native handler first walks argument 0 with the table iterator
    // (00B67190) and collects each element's "Ptr" into a std::vector; the
    // collection is the caller's job here. The second loop is this one.
    void* group = nullptr;
    for (std::size_t i = 0; i < entity_count; ++i) {
        void* entity = entities[i];
        if (entity == nullptr) {
            continue;
        }
        auto* existing = *reinterpret_cast<void**>(
            static_cast<unsigned char*>(entity) + kEntityAiGroupOffset);
        if (existing != nullptr) {
            host.ai_group_remove_entity(existing, entity);
        }
        if (group == nullptr) {
            // 00A38CAE allocates 5660h bytes; a null allocation leaves the group
            // null and the loop keeps going, which is why the trailing store
            // below can be reached with no group at all.
            group = host.ai_group_construct(entity);
        } else {
            host.ai_group_add_entity(group, entity);
        }
    }
    // 00A38D0x stores 0 into group+5648h without a null test. An empty argument
    // table, or a failed allocation, reaches that store with a null pointer;
    // this reconstruction declines to reproduce the fault.
    if (group != nullptr) {
        auto* flag = static_cast<unsigned char*>(group) + kAiGroupGroupingEnabledOffset;
        *flag = 0u;
    }
    return kNoResults;
}

} // namespace bsp
