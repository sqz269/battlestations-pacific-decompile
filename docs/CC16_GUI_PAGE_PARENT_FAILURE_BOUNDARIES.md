# Page and Group failure boundaries

At Source epoch da99ff56f, the implemented Page/Group Widget-base path already
provides negative failure coverage. Its retained Widgets provider implements
both raw-node lifetime and timed-storage interfaces. AA9730 therefore calls
`raw_widget_lifetime_failed()` on escape; that marks Source failure and does
not execute the plain-binding Native cleanup. Group scalar deletion skips its
pool return after an escape. At Layer's final base call, state is already-1,
so that catch performs no Native cleanup at this phase.

An additional three-argument parent guard is not required for this implemented
base-phase path. It could bypass the existing raw failure notification. The
standalone prepared parent API remains applicable to plain bindings and other
ancestors lacking equivalent coverage. This qualifies earlier general parent
prerequisite wording in the Source66/67 author records; those immutable proofs
and Source pins remain historical evidence. It does not activate the Page path
or certify earlier Layer scene/name cleanup phases.

The manager boundary is separate. TU-private GuiCleanup is armed at state4
before Page callbacks; its noexcept unwind reaches string-list head-link stores
before any provider callback. The outer singleton catch can also reach BD0220.
Host pre-drain retention and the later drain catch cannot prevent those earlier
in-drain projections.

A failed-Page query is insufficient for whole manager UNKNOWN coverage.
Page-app admission rejections and entered Page failures imply existing sticky
retention, but an absent delegate, manager validation, or manager-owned helpers
can escape without setting Page failure. The initial five-file query sketch
is a partial candidate, not a closed whole-manager solution. The proposed next
whole selected route needs one genuine cold, retained Source attempt armed
before throwing admission and shared through both cleanup boundaries. It must
not infer exception origin, add cleanup/reset, or replace the existing terminal
retention policy. Its exact API and ownership remain under review.

The report records the sealed69-artifact audit and its coverage addendum.
No C++/CMake, Ghidra, ledger, build, game or runtime activation changed. Original
ABI/Native origin, full manager exception behavior, normal disposition and
production Icon providers remain open.
