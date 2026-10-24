#pragma once
#include "bsp/native_gui_widget_child_links.hpp"
#include "bsp/native_gui_widget_property_interaction.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_node_parenting.hpp"
#include <deque>

namespace bsp {
// Genuine current calls over the SAME raw visitor/widget storage. Numeric
// targets select source implementations, never host function pointers. The
// factory must use the real AA6560 class factories and their matching pools.
// These calls must not retire a receiver before its remaining native reads.
class NativeGuiWidgetPropertyChildrenCalls {
public:
    virtual ~NativeGuiWidgetPropertyChildrenCalls() = default;
    virtual void enumerate_keys_18(void* visitor, std::uint32_t target, void* block) = 0;
    virtual void* create_widget_00aa6560(std::uint32_t type, const void* source) = 0;
    virtual void widget_created_74(void* widget, std::uint32_t target) = 0;
    virtual void enter_key_04(void* visitor, std::uint32_t target,
        std::uint32_t tag, std::uint32_t payload) = 0;
    virtual void read_widget_18(void* widget, std::uint32_t target, void* visitor) = 0;
    virtual void leave_key_08(void* visitor, std::uint32_t target) = 0;
    virtual void widget_loaded_78(void* widget, std::uint32_t target) = 0;
};
struct NativeGuiWidgetPropertyChildrenContext {
    NativeStringRawPoolContext& strings;
    const NativeGuiWidgetTypeKeyContext& types;
    NativeInstanceGeometryAccess& models; // existing SAME Model companion lifecycle
    const NativeNodeRawConstants& model_constants;
    NativeNodeParentingRuntime& parenting;
    NativeGuiWidgetPropertyChildrenCalls& calls;
};
// Diagnostics only. Each child preserves its independently acquired native
// creator on a later failure. The child's actual+4C becomes its sole native
// owner; these pointers never add references or another tree/field authority.
struct NativeGuiWidgetPropertyChildAcquired {
    std::uint32_t type{};
    void* widget{};
    void* raw_model{};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_reference{};
    bool constructor_returned{}, model_bound{}, model_published{}, linked{};
};
struct NativeGuiWidgetPropertyChildrenAcquired {
    enum class Phase { fresh, entered, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t active_call_site{};
    bool name_live{}, name_cleanup_entered{};
    std::deque<NativeGuiWidgetPropertyChildAcquired> children;
    NativeGuiWidgetPropertyChildrenAcquired() = default;
    NativeGuiWidgetPropertyChildrenAcquired(const NativeGuiWidgetPropertyChildrenAcquired&) = delete;
    NativeGuiWidgetPropertyChildrenAcquired& operator=(const NativeGuiWidgetPropertyChildrenAcquired&) = delete;
    ~NativeGuiWidgetPropertyChildrenAcquired() noexcept;
};

// AAA710 suffix [AAACD4,AAAEB4): enumerate250 variants in the SAME earlier
// 820h scratch, construct genuine default raw widgets and actual188h Models,
// append their SAME raw list cells, reparent and dispatch current74/18/78.
// The final machine epilogue/FH3/SEH/private stack are not this C++ ABI.
//
// The scratch is an existing initialized preimage, not zeroed storage. Only
// the250 tag words and count are initialized; enumeration payloads/proxies stay
// native preimages until real readers write them. Valid native block bounds,
// terminated keys, rings and all live transitive providers/backing are caller
// obligations. No logical widget cast, synthetic list progress or fallback
// factory is supplied. Calls and lifetime/slot reuse must be serialized.
//
// SAME original93/118/122 and other derived factory custody remains mandatory
// when the factory returns a derived object. Reentrant calls require different
// retained scratch/diagnostic frames. Entered failure keeps partial effects,
// creator companions and graph; prohibit retry and automatic dependency drain.
void continue_native_gui_widget_property_children_00aaacd4(void* actual_widget,
    void* actual_visitor, NativeGuiWidgetPropertyScratch&,
    NativeGuiWidgetPropertyChildrenContext&, NativeGuiWidgetPropertyChildrenAcquired&);

// Full normal source schedule through all three existing/raw stages. Does not
// supply a Page application or make the mandatory derived factories available.
void read_native_gui_widget_properties_and_children_00aaa710(void* actual_widget,
    void* actual_visitor, NativeGuiWidgetPropertyScratch&,
    NativeGuiWidgetPropertyInteractionContext&, NativeGuiWidgetPropertyChildrenContext&,
    NativeGuiWidgetPropertyChildrenAcquired&);
} // namespace bsp
