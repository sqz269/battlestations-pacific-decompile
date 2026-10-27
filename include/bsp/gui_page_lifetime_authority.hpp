#pragma once
#include "bsp/gui_widget_owner.hpp"
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <utility>

namespace bsp {

// Host ownership capability for one EXISTING sole GuiLayoutPage allocation.
// This is not an actual124h native page, its +04 reference count, a registry,
// loader, destructor transport or a second owner of its widgets/layout tree.
// The borrowed runtime and every native/type/provider domain must outlive this
// authority, its aliases, and the page's eventual explicit terminal retirement.
// Calls, token changes, tree mutation and native drain are externally serialized.
class GuiPageLifetimeAuthority final {
public:
    // Allocate metadata BEFORE consuming the unique_ptr. On admission/allocation
    // failure the caller keeps the SAME page. Success moves its sole ownership;
    // no page/root/widget is allocated, copied, registered or constructed here.
    explicit GuiPageLifetimeAuthority(std::unique_ptr<GuiLayoutPage>& sole_page,
        GuiWidgetOwnerRuntime& widgets) : state_(std::make_shared<State>(widgets)) {
        if (!sole_page)
            throw std::invalid_argument("page authority requires its sole existing owning page allocation");
        state_->page = std::move(sole_page);
    }
    ~GuiPageLifetimeAuthority() noexcept {
        // No destructor-time native cleanup. A future page host must complete
        // resource/borrower retirement and take the sole allocation explicitly.
        if (state_->page) std::terminate();
    }
    GuiPageLifetimeAuthority(const GuiPageLifetimeAuthority&) = delete;
    GuiPageLifetimeAuthority& operator=(const GuiPageLifetimeAuthority&) = delete;
    GuiPageLifetimeAuthority(GuiPageLifetimeAuthority&&) = delete;
    GuiPageLifetimeAuthority& operator=(GuiPageLifetimeAuthority&&) = delete;

    // Borrow only. Destructive changes to page/root/child ownership, explicit
    // widget deletion or any terminal retirement must first pass the gate below.
    // Native scene/model release is allowed while aliases remain: this is how
    // material borrowers can be genuinely retired before dropping their guards.
    GuiLayoutPage& page() const {
        require_page();
        return *state_->page;
    }
    GuiWidgetOwnerRuntime& widgets() const noexcept { return state_->widgets; }

    // Owns the SAME sole page/tree and aliases exactly &canonical_widget.
    // Pointer membership is proven by traversing owned unique_ptr children,
    // never inferred from a name, parent pointer, raw +04 count or no-op deleter.
    // A pending child still owned outside this page is deliberately rejected.
    // Adds no native retain/count and creates no duplicate widget/owner map.
    std::shared_ptr<const void> retain_widget(GuiWidgetOwner& canonical_widget) const {
        require_page();
        auto& layout = canonical_widget.layout();
        if (&canonical_widget.runtime() != &state_->widgets ||
            &state_->widgets.owner(layout) != &canonical_widget ||
            !contains(state_->page->root.get(), &layout))
            throw std::invalid_argument("page token requires its same owned canonical widget descendant");
        return std::shared_ptr<const void>(state_, &canonical_widget);
    }
    std::size_t retained_alias_count() const noexcept {
        return static_cast<std::size_t>(state_.use_count() - 1);
    }
    bool has_page() const noexcept { return bool(state_->page); }
    bool can_terminal_retire() const noexcept {
        return has_page() && retained_alias_count() == 0;
    }
    // Pure preflight, before removing a page registry pointer, destroying a
    // widget/layout, or transferring any owned root/child out of this authority.
    // Does not release material guards, change native +04, or run GUI virtuals.
    void require_terminal_retirement() const {
        require_page();
        if (retained_alias_count() != 0)
            throw std::logic_error("page terminal retirement requires all same-widget aliases released");
    }
    // Sole ownership transfer, not a reconstructed native terminal call. The
    // caller must preserve this exact allocation through its genuine retirement
    // transport, including any entered failure. Do not silently drop the result.
    std::unique_ptr<GuiLayoutPage> take_for_terminal_retirement() {
        require_terminal_retirement();
        return std::move(state_->page);
    }
private:
    struct State {
        explicit State(GuiWidgetOwnerRuntime& actual) noexcept : widgets(actual) {}
        ~State() noexcept { if (page) std::terminate(); }
        GuiWidgetOwnerRuntime& widgets;
        std::unique_ptr<GuiLayoutPage> page;
    };
    std::shared_ptr<State> state_;
    void require_page() const {
        if (!state_->page)
            throw std::logic_error("page authority has no owning allocation after terminal transfer");
    }
    static bool contains(const GuiLayoutWidget* root, const GuiLayoutWidget* sought) noexcept {
        if (!root) return false;
        if (root == sought) return true;
        for (const auto& child : root->children)
            if (contains(child.get(), sought)) return true;
        return false;
    }
};
} // namespace bsp
