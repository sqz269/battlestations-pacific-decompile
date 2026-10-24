# Page lifetime authority prerequisite, 2026-10-24

`GuiPageLifetimeAuthority` is a host ownership capability for a future native
page loader. It consumes the caller's existing sole
`unique_ptr<GuiLayoutPage>` after metadata allocation succeeds. Its shared
control state then holds that same unique_ptr: there is one page allocation,
one root and one tree of unique_ptr children. Nothing constructs or activates
a page, GUI manager, screen manager, scene node, model, or native type.

`retain_widget` returns `shared_ptr<const void>` whose `get()` is exactly the
canonical `GuiWidgetOwner` address. The same borrowed widget runtime must resolve
that owner, and the exact layout pointer must occur in the authority's owned
root/child tree. A name, parent pointer, raw widget reference count, or borrowed
page pointer cannot substitute for that ownership. The token owns the shared
state containing the sole page; it is not a no-op-deleter alias or another
native widget reference.

The borrowed `GuiWidgetOwnerRuntime`, native/provider domains, and authority
must stay alive through all tokens and the eventual actual retirement. This
service does not own or reconstruct those external domains. A host must not
call widget/layout terminal deletion, clear/replace owned children, or bypass
the ownership gate while tokens remain.

## Material retirement order

Scene/model release may run while tokens remain. This is distinct from terminal
page/widget/layout destruction, and is necessary to release materials borrowing
widget parameter fields without making a lifetime cycle.

1. Retire genuine queued render uses and release the applicable scene/model and
   material references, keeping the page tree, widgets and fields alive.
2. Release each adapter's material-borrow guard after its actual native-owner
   conditions prove that no material still borrows those fields. The Icon
   adapter uses a null primary node and its raw Icon +04 creator-only count;
   the authority does not invent or decrement that count.
3. `require_terminal_retirement` rejects any remaining aliases before page
   unregistration or terminal calls. `take_for_terminal_retirement` performs
   the same check and returns the exact original unique_ptr, leaving the
   authority empty. The real page host supplies its native terminal transport
   and retains all resources on entered failure. This service does not drop
   or scalar-delete that page automatically.

Destroying an authority with its page still inside is a contract failure. It
never performs hidden destructor-time GUI retirement. No native +04 count,
reference-credit, hierarchy, transform, owner map or game state is added.

## Existing owner and loader boundaries

`GuiPageRegistry` owns `vector<unique_ptr<GuiLayoutPage>>` in
`gui_layout_loader.hpp:377-399`. Both AA52A0 source registration overloads move
the sole page allocation into that vector. `GameFrontendHost::Impl` currently
holds this registry by value. Neither its borrowed lookup result nor the
registry's `pages()` borrow is an owning shared page capability. Those legacy
pages cannot use this authority until an explicit sole-owner transfer is
authored. This change leaves that registry/frontend intact.

Current `load_gui_page_00aa5840` keeps its root in a local unique_ptr until after
property/child construction (`gui_layout_loader.cpp:751-788`). A future loader
must establish that exact root's ownership inside the adopted page before
material callbacks request a page token; the authority cannot own a root that
still belongs to another local allocation handle. Borrowed script/property
tables must likewise have their owning page fields or an independently retained
provider established before those callbacks; the authority invents no owners
for an external table or other borrowed provider.

Current `build_widget_children_00aaa710` invokes the child factory before
appending the child unique_ptr to the parent's owning list (lines674-684).
It invokes current74 after that publication, then properties and current78.
An Icon token must therefore be acquired at its later rebuild/material-binding
entry, not the earlier make_type callback for an out-of-tree pending child.
The Icon worker confirmed current74 itself needs no material-field borrower.
Parent pointers alone do not waive either ownership boundary.

Raw `NativeGuiPageConstructionAcquired` explicitly owns no page, and the
available AC6600 prefix does not establish a completed page. Raw native page
registry helpers accept actual manager/page allocations; a `GuiLayoutPage`
cannot be passed as that actual124h object. This authority adds no completion
claim for those native paths and never instantiates 004C12B0 or its chain.

The new API is C++ host ownership, with no recovered original ABI or new
reconstruction/annotation batch. Evidence remains in the existing page loader,
widget owner and material-binding sources. No Ghidra mutation or ledger change
is needed for this capability.

## Validation

The header is compiled through a temporary include in the worker's already
owned `gui_widget_owner.cpp` during `scripts/build.ps1`; that temporary include
was restored before delivery. MSVC Win32 Release compiled and linked, and both
existing CTest checks passed: `reconstructed_math` 0.54s, `tool_tests` 14.29s,
2/2 passed (14.90s total). The ignored build log is
`local/oct24-gui-page-authority-build.log`. No focused lifetime fixture or new
test suite was added. No page instantiation, native page
delivery, OFF/ON comparison, screenshot or game validation is claimed.
