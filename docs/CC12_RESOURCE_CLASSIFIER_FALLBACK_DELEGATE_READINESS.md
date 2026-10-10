# Fallback type delegate readiness

The exact delegated initializer at B86A00 is now bounded and byte-verified:
one complete 125-byte body, ECX receiver, preserved ESI, no stack arguments,
ordinary RET. Current typed metadata reports an exact owned entry, no-return
false and nonthunk. This is Native evidence and Source readiness, not a newly
implemented provider, production backing or binary replacement.

The earlier startup entry supplies ECX=0109021C. That entry's independent
ten-byte audit is separate. B86A00 uses a process guard at 0109020D; the guard
is not a field of its receiver. It writes receiver+0Ch with numeric literal
token D631F4. On that canonical receiver, the observed four words therefore
have own/Scene/root/name roles at 0109021C/20/24/28. No full class/vtable layout
or literal text/extent was read or admitted from those roles.

The cold path publishes the fallback guard and name first. It then reads the
actual Scene guard 0109020C. When Scene is cold, it publishes that guard and
the existing D631E4 Scene name token, calls genuine BEA780 on canonical root
0109DB84, reloads its current own ID into Scene+4, and calls genuine 6FAC20.
It increments returned counter+4 with wrapping arithmetic before publishing
the old ID into Scene+0. This corrects the decompiler's reversed-looking
Scene-own assignment: assembly, not the expression grouping, establishes the
actual increment-before-own order.

The fallback tail loads current Scene own ID, immediately stores receiver+4,
then loads current Scene root ID and stores receiver+8. These are alternating
load/store operations; the camera CRT fragment's paired captures must not be
copied here. It calls the same real counter getter, increments counter+4 and
publishes the captured old value at receiver+0 last. A nonzero fallback guard
returns without any destination write. The guard remains sticky after any
fallible dependency and earlier stores are not rolled back.

Root checked pseudocode register inputs against the complete assembly and
compared all 125 original/live bytes and every instruction start. Five full
typed raw responses pass the current strict identity validator at modification
5. Each of the three callsites has override NONE, matching ordinary default
and effective call flow, no fallthrough override and actual callees recorded
as returning. No callee body, string data, handler, table or adjacent function
was newly opened. The SDK loaded Java CodeSource remains unattested.

The genuine current Source Scene initializer B869C0 already performs the
observed inner guard/name/root/counter schedule on actual Scene storage. Its
consume_id calls the same retained TypeIdCounterLifetime and publishes its
increment before returning the old value. Existing VFS type composition
shares that counter/root domain. These complete current implementations and
whole Source files are pinned; no header-only provider or copied ID is used.

A minimal ordinary Source provider can borrow the actual 0109020D guard,
stable actual four-word receiver, exact numeric name token, canonical current
Scene storage, genuine Scene provider and the same retained counter/root
domain. It must preserve the alternating parent load/store tail, wrap before
own-ID publication and retain sticky partial writes on Source exceptions.
Calling the real Scene Source provider can represent the observed inline
schedule, under explicit same-storage/domain/lifetime contracts. The native
ECX/ESI/RET and original exception/fault behavior remain separate from any
new ordinary C++ interface.

Current GameNativeTypeStorage does not explicitly declare the fallback guard
or descriptor, and the admitted classifier still borrows the three current
tokens. A future Source provider does not supply their production owner,
application consumer attachment or original CRT placement/order. Concrete
backing/lifetimes and the consumer's other genuine loading/forwarding/deletion
gates remain outstanding. Root changed no C++, CMake, ledger, GPR or game
installation and ran no build, probe, tests or game for this readiness audit.

Report: [exact delegate evidence and Source pins](../reports/cc12_resource_classifier_fallback_delegate_readiness.json).
