# GUI Models born in the raw name domain

`GameNativeGuiNodeApplication` now configures its SAME Model environment with
raw node-name transport before constructing any GUI Model. It retains the
canonical01090054 pool, type/bootstrap instances, actual-owner registry,
Node lifetime runtime, constants and hierarchy. No live environment is switched,
and no second Model, pool, registry or name owner is created.

The need is concrete: full raw Text construction/sections/retirement require
the same raw Model/node cleanup domain with `actual_names==nullptr`. The first
native Text application instead selected `actual_names=&raw.strings`, the
existing `NativeStringStorage` adapter. That worked for the earlier source
route but could not admit the existing raw Text bodies without two competing
transport modes.

`NativeModelRawNameServices` borrows the renderer's actual node constants and
the same existing storage adapter for source callers' temporary NativeString
headers. The Model environment's optional pointer is fixed from birth. The
typed B75030 source entry forwards the address of its SAME live8B argument to
the genuine raw B75030 overload. The raw overload still requires raw runtime
cleanup, null `actual_names`, unused prepared storage and identical bound
constant cells; none of those admission checks is weakened.

`GuiWidgetOwnerRuntime` chooses that same temporary-name adapter for primary
and tracked auxiliary Models. Its preflight rejects another adapter or mixed
mode before allocating a Model. Existing instance-geometry source admission
recognizes exactly two explicit modes: its prior identical adapter, or the new
born-raw mode with identical temporary adapter and raw runtime. Mixed modes are
rejected. Constructors and raw node cleanup use the runtime's actual pool,
publication, manager and shutdown-gate domain.

Only GUI's environment selects this mode. Existing callers whose optional
service pointer is null keep their prior source entry. Native ABI remains
B75030: ECX actual184h object in its188h slot, stack borrowed8B name, EAX same
object, RET4. The source adapter itself is not a binary ABI replacement. The
temporary `NativeStringStorage::release` interface retains its existing noexcept
qualification; actual Model/node names use the genuine raw path. No original
FH3/SEH, unusual failure transport or private-stack compatibility is added.

This removes a Model-domain prerequisite for raw Text field/state composition.
It does not replace Step1's eight-byte Text identity with the full raw Text,
admit a native Page, or construct a GUI manager. Build and normal ON/OFF evidence
are recorded in `OCT24_GUI_RENDER.md` when the batch is integrated.
