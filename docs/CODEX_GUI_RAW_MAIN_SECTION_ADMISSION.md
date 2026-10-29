# Compound MainSection admission

The immutable prefix operation now has a distinct `geometry_main_section` choice
that binds both the existing Geometry context and a const MainSection lifetime
Frame. It is not an independently selectable extra pointer beside geometry-only
permission. Existing None/GeometryOnly constructors retain their old kind and
geometry identity and leave the new pointer null.

This first API step grants no ownership, constructor, scalar or cleanup authority.
Current shared guards deliberately reject the new kind until the exact retained
helper and staged checks are coupled. No existing owner selects it. The new helper
is a C++ interface hypothesis, not a recovered native class or Original ABI.

Implementation must reserve the whole graph before raw122/pools, admit exact
untouched metadata before379 and before MainSection, and require the real83f
private created-normal witness plus a canonical sole Section/Material graph for
scalar permission. A pending, failed or skipped graph cannot fall back to an
earlier operation. The faithful first ABA8D0 sequence creates the main section
before Font/content; Font108/1EC remain empty in this first domain.

Root owns the shared geometry/reference/retirement/prefix/Widget changes. The two
late empty-Font fallbacks must use exact bit6-return metadata for the compound
domain; no returned header or erased Mesh/Section/Material companion is read.
Ordinary Model/type checks, false predicate before188, initial DEFAULT validation
and native body/cleanup schedules remain intact. The new material cache backing
uses the existing typed recursive walker and remains alive through actual renderer
verification; a base sizeof or fixed-array truncation is not a transitive proof.

Independent actual-diff review accepts the immutable API. Win32 build and both
existing checks pass (15.28s). No caller, runtime fixture, production switch,
native-body credit or ABI proof follows from this declaration.
