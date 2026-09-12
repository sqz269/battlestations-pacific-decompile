#include "bsp/gui_pointer_hit_runtime.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

// AA8BDC..AA8C36. The fields are borrowed from the actual transform; only
// the native frame's pivot products, absolute position and child origin spill.
void child_origin(const GuiWidgetTransform& widget, const GuiWidgetPoint& origin,
    const volatile double* bias, GuiWidgetPoint& result) {
    const float* pivot = &widget.pivot_x;
    const float* extent = &widget.size.width;
    const float* position = &widget.position.x;
    const float* parent = &origin.x;
    float* output = &result.x;
    float products[2];
    float absolute[3];
    __asm {
        mov eax, pivot
        mov edx, extent
        fld dword ptr [eax]
        fmul dword ptr [edx]
        fstp dword ptr products[0]
        fld dword ptr [eax+4]
        fmul dword ptr [edx+4]
        fstp dword ptr products[4]
        mov eax, position
        mov edx, parent
        fld dword ptr [eax]
        fadd dword ptr [edx]
        fstp dword ptr absolute[0]
        fld dword ptr [eax+4]
        fadd dword ptr [edx+4]
        fstp dword ptr absolute[4]
        fld dword ptr [eax+8]
        fadd dword ptr [edx+8]
        fstp dword ptr absolute[8]
        mov edx, output
        fld dword ptr absolute[0]
        fsub dword ptr products[0]
        fstp dword ptr [edx]
        fld dword ptr absolute[4]
        fsub dword ptr products[4]
        fstp dword ptr [edx+4]
        fld dword ptr absolute[8]
        mov eax, bias
        fsub qword ptr [eax]
        fstp dword ptr [edx+8]
    }
}

// AA8CA1..AA8CD6: FST preserves the unpopped left/top value for its addition.
void initial_bounds(const GuiWidgetPoint& origin, const GuiWidgetSize& size,
    float* bounds) {
    const float* position = &origin.x;
    const float* extent = &size.width;
    __asm {
        mov eax, position
        mov ecx, extent
        mov edx, bounds
        fld dword ptr [eax]
        fst dword ptr [edx]
        fadd dword ptr [ecx]
        fstp dword ptr [edx+8]
        fld dword ptr [eax+4]
        fst dword ptr [edx+4]
        fadd dword ptr [ecx+4]
        fstp dword ptr [edx+12]
    }
}

// AA8CDC..AA8DE1, retaining the mixed SSE/x87 zero gates, binary32 spills,
// x87 operand order, live pointer loads and unspilled containment comparisons.
// bounds = left, top, right, bottom. Its right slot becomes the native scratch
// after the right edge is held in ST0. Extra fields are actual widget30..44.
bool candidate(float* bounds, const float* extra, const float* depth,
    const volatile float* pointer_x, const volatile float* pointer_y,
    const volatile float* previous_depth) {
    float scratch;
    bool selected;
    __asm {
        mov esi, extra
        mov edi, bounds
        fld dword ptr [esi]
        movss xmm1, dword ptr [esi+8]
        fstp scratch
        xorps xmm0, xmm0
        fld scratch
        ucomiss xmm1, xmm0
        fld st(0)
        lahf
        test ah, 44h
        fadd dword ptr [edi]
        fstp dword ptr [edi]
        fld dword ptr [esi+4]
        fstp scratch
        fld scratch
        movss scratch, xmm1
        fld st(0)
        fadd dword ptr [edi+4]
        fstp dword ptr [edi+4]
        fld dword ptr [edi+8]
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [edi+8]
        fadd dword ptr [edi+12]
        fstp dword ptr [edi+12]
        fld dword ptr [edi+8]
        jnp first_limit_done
        fld scratch
        fcomip st(0), st(1)
        ja discard_one
    first_limit_done:
        movss xmm1, dword ptr [esi+12]
        fld dword ptr [edi]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        movss dword ptr [edi+8], xmm1
        jnp second_limit_done
        fld dword ptr [edi+8]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        ja discard_two
    second_limit_done:
        movss xmm1, dword ptr [esi+16]
        fld dword ptr [edi+12]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        movss dword ptr [edi+8], xmm1
        jnp third_limit_done
        fld dword ptr [edi+8]
        fcomip st(0), st(1)
        ja discard_three
    third_limit_done:
        movss xmm1, dword ptr [esi+20]
        fld dword ptr [edi+4]
        ucomiss xmm1, xmm0
        lahf
        test ah, 44h
        movss dword ptr [edi+8], xmm1
        jnp fourth_limit_done
        fld dword ptr [edi+8]
        fxch st(1)
        fcomi st(0), st(1)
        fstp st(1)
        ja discard_four
    fourth_limit_done:
        mov eax, pointer_x
        fld dword ptr [eax]
        fcomi st(0), st(3)
        fstp st(3)
        jc discard_four_containment
        fxch st(3)
        fcomip st(0), st(2)
        fstp st(1)
        jc discard_two_containment
        mov eax, pointer_y
        fld dword ptr [eax]
        fcomi st(0), st(2)
        fstp st(2)
        jc discard_two_containment
        fcomip st(0), st(1)
        fstp st(0)
        jc rejected
        mov eax, depth
        fld dword ptr [eax]
        mov eax, previous_depth
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        jbe rejected
        mov selected, 1
        jmp finished
    discard_four:
        fstp st(0)
    discard_three:
        fstp st(1)
    discard_two:
        fstp st(0)
        fstp st(0)
        jmp rejected
    discard_four_containment:
        fstp st(2)
        fstp st(1)
        fstp st(0)
        fstp st(0)
        jmp rejected
    discard_two_containment:
        fstp st(1)
    discard_one:
        fstp st(0)
    rejected:
        mov selected, 0
    finished:
    }
    return selected;
}
} // namespace

GuiPointerHitRuntime::GuiPointerHitRuntime(GuiPointerHitRuntimeServices services)
    : services_(services) {}

void GuiPointerHitRuntime::hit_test_widget_00aa8bd0(GuiWidgetOwner& widget,
    const GuiWidgetPoint& origin) {
    require(&widget.runtime() == &services_.frames.widgets(),
        "GUI recursive hit requires the same canonical widget runtime");
    auto& layout = widget.layout();
    auto& transform = layout.transform;
    GuiWidgetPoint next_origin;
    child_origin(transform, origin, &services_.z_bias_00d7a258, next_origin);
    // The existing projection owns the same children in two corresponding
    // vectors. Membership/order must remain stable through this borrowed walk.
    const auto count = transform.children.size();
    const auto child_at = [&](std::size_t index) -> GuiWidgetOwner& {
        require(transform.children.size() == count && layout.children.size() == count,
            "GUI recursive hit child membership changed during traversal");
        require(index < count && layout.children[index] &&
            transform.children[index] == &layout.children[index]->transform,
            "GUI recursive hit requires corresponding actual child storage");
        return services_.frames.widgets().owner(*layout.children[index]);
    };
    for (std::size_t index = 0; index < count; ++index) {
        auto& visible_child = child_at(index);
        if (visible_child.implementation().is_visible38(visible_child))
            hit_test_widget_00aa8bd0(child_at(index), next_origin); // fresh node payload
        (void)child_at(index); // native iterator check after the callback
    }
    if (!transform.mouse_hit || widget.scene_flags().hidden) return;
    float bounds[4];
    initial_bounds(next_origin, transform.size, bounds); // size is current after children
    services_.frames.align_bounds64(widget, bounds[0], bounds[1], bounds[2], bounds[3]);
    if (candidate(bounds, widget.extra_fields().fields_30_44, &next_origin.z,
        &services_.pointer_x_00f8bc74, &services_.pointer_y_00f8bc78,
        &services_.hit_depth_00f8bc7c)) {
        services_.hovered_00f8bc70 = &layout;
        services_.hit_depth_00f8bc7c = next_origin.z;
    }
}

void GuiPointerHitRuntime::hit_test_00aa2f10(GuiResourceOwner& resources) {
    auto& fields = resources.pointer_fields();
    require(fields.position_5c.has_value(),
        "GUI manager hit requires the actual AA5D70 pointer position producer");
    services_.pointer_x_00f8bc74 = (*fields.position_5c)[0];
    services_.pointer_y_00f8bc78 = (*fields.position_5c)[1];
    const float initial = services_.initial_depth_00ce4970;
    services_.hovered_00f8bc70 = nullptr;
    services_.hit_depth_00f8bc7c = initial;
    const auto& pages = resources.pages().pages();
    const auto* storage = pages.data(); // native begin captured once; no page snapshot
    const auto page_at = [&](std::size_t index) -> GuiWidgetOwner& {
        require(pages.data() == storage && index < pages.size(),
            "GUI manager hit page iterator was invalidated");
        require(pages[index] && pages[index]->root,
            "GUI manager hit requires an actual registered page root");
        return services_.frames.widgets().owner(*pages[index]->root);
    };
    for (std::size_t index = 0;; ++index) {
        require(pages.data() == storage && index <= pages.size(),
            "GUI manager hit page iterator was invalidated");
        if (index == pages.size()) break; // native end reloaded on each pass
        auto& visible_page = page_at(index);
        if (visible_page.implementation().is_visible38(visible_page)) {
            if (!fields.exclusive_page_6c ||
                &page_at(index).layout() == fields.exclusive_page_6c) {
                const GuiWidgetPoint origin{0.0f, 0.0f, 0.0f};
                hit_test_widget_00aa8bd0(page_at(index), origin);
            }
        }
        (void)page_at(index); // fresh native end check before increment
    }
    auto* hovered = services_.hovered_00f8bc70;
    if (!hovered || hovered->transform.mouse_block) return;
    const auto mouse = services_.frames.input_source().device(1); // fresh backend after walk
    require(static_cast<bool>(mouse), "GUI manager hit current28 requires an actual mouse");
    if (mouse.activity_current28()) {
        hovered = services_.hovered_00f8bc70; // activity can replace the publication
        require(hovered != nullptr, "GUI manager hit current68 requires current hovered widget");
        services_.frames.dispatch_current68(services_.frames.widgets().owner(*hovered), nullptr);
    }
}
} // namespace bsp
