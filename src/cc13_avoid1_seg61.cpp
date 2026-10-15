// cc13_avoid1, packet B: the spawn request completion 00947A30. See
// include/bsp/cc13_avoid1_seg61.hpp and docs/CC13_AVOID1.md.
#include "bsp/cc13_avoid1_seg61.hpp"

#include "bsp/native_string_compare.hpp"

namespace bsp::cc13_avoid1 {

void cc13_spawn_request_complete_00947a30(Cc13SpawnRequestCompletionNative& request,
                                          Cc13SpawnCompletionHost& host) {
    if (request.callback.size == 0) return;                       // 00947A5C
    if (request.callback.data[0] == '[') {                        // 00947A6E: no data null test
        if (equal_native_string_header_00425850(&request.callback, "[supportmanager]")) {  // 00947A7A
            Cc13NativeDwordVector by_value;                       // the argument slot at ESP
            host.member_vector_copy_00947810(by_value, request.members);  // 00947A93
            void* owner = host.support_orders_owner_00e198c4_bc();       // 00947A98..00947A9E
            host.support_member_orders_0065d070(owner, by_value);        // 00947AA4
            return;
        }
        host.ai_group_from_script_00a16ef0(request.callback, request.members);  // 00947AC4
        return;
    }
    // 00947ADC..00947AF6: operator new(0Ch), zeroed when non-null.
    auto* args = static_cast<Cc13LuaVariantVectorNative*>(host.allocate_00bf681b(0x0c));
    if (args != nullptr) {
        args->first = nullptr;
        args->last = nullptr;
        args->end = nullptr;
    }
    for (std::uint32_t i = 0;; ++i) {                             // 00947B00
        const std::uint32_t* first = request.members.first;
        if (first == nullptr ||
            i >= static_cast<std::uint32_t>(request.members.last - first)) {
            break;                                                // 00947B08 / 00947B17
        }
        if (request.members.first == nullptr ||
            i >= static_cast<std::uint32_t>(request.members.last - request.members.first)) {
            host.invalid_parameter_00bf6713();                    // 00947B2C, checked operator[]
        }
        LuaVariant variant;
        host.variant_construct_entity_00884a90(variant, request.members.first[i]);  // 00947B41
        try {
            host.variant_vector_push_back_006edf00(args, variant);  // 00947B51, EH state 0
        } catch (...) {
            host.variant_destruct_00884af0(variant);
            throw;
        }
        host.variant_destruct_00884af0(variant);                  // 00947B62
    }
    void* runner = host.lua_runner_00e188a8_1a08();               // 00947B70..00947B75
    host.call_named_00887e50(runner, 0, request.callback, args, 0, -1);  // 00947B81
    if (args != nullptr) {                                        // 00947B86
        host.variant_vector_resize_006b88e0(args, 0);             // 00947B8D
        host.free_array_00bf6989(args->first);                    // 00947B95
        host.free_record_00bf65ac(args);                          // 00947B9B
    }
}

}  // namespace bsp::cc13_avoid1
