# Game-resource factory in executable startup

Addresses: 008f81f0, 008f840b, 008f8414, 007175d0, 008f8449, 00bd0400.

The executable's `StartupHost::publish_game_resource_factory` now calls the
recovered actual-storage getter instead of recording an unimplemented phase.
`GameSingletonHost` owns stable, separate source cells for native E19B90 and
WinMain F8D31C, plus a factory context borrowing its existing raw manager cell.
The context is installed in the shared raw deletion bindings before startup
can invoke the getter and register the factory's +4 lifetime subobject.

The original instruction sequence is `008F840B CALL 007175D0`, an application
address calculation, then `008F8414 MOV [00F8D31C],EAX`. The source preserves
getter-then-alias assignment before application construction. An exception from
the getter does not assign the alias or log the startup phase as implemented.
This binds the previously reconstructed owner; it adds no new original body.
Names are descriptive hypotheses. The new source cells are not installed at
native virtual addresses, and the C++ host/context interfaces are not native ABI
replacements. The native getter takes no argument and returns its pointer in EAX.

Normal `008F8449` shutdown uses the same raw manager and borrowed factory context.
The admitted CFD84C secondary profile dispatches 00716520, which adjusts -4 and
deletes the primary allocation through 00716560. Deletion clears E19B90 while
leaving F8D31C's alias bits intact. Diagnostic logs record the actual source
publication and alias at publication and after the drain, without dereferencing
the retained alias after deletion.

The current observer runtime's field order, initializer, bindings and
post-drain accounting remain intact. `GameStartupHost` retains `GameSingletonHost`
and its context through both explicit normal shutdown and destructor fallback,
before deleting the host. As in the native getter, registration failure may
leave an allocated, published owner that was not registered; the fallback drain
cannot promise to clear that allocation. No new allocation rollback is added.
Also, the existing executable closes its file log before host destruction, so
fallback cleanup following a caught startup exception may be recorded only on
stdout. These existing boundaries must remain distinct from a successful normal
run's publication and shutdown evidence.

The strict MSVC Win32 build and both existing CTests passed at source commit
`486771757d98edf57d507fe5ff80e991ee6a3332`. A bounded executable run at that commit
then presented 60 frames at 1280x720, exited zero and drained six registered
slots. Publication logged matching nonnull owner/alias pointers. Normal shutdown
logged null manager/effect/factory publications and the exact original factory
alias bits. The run used an isolated settings directory. The exact-commit build
and runtime receipts are retained under `local/native-game-resource-factory-process-bc`
(`attempt02-build-proof.json` and `runtime-attempt03/result.json`). Later merged
candidates require their own build and runtime validation. No permanent test is
added; compilation alone does not prove publication or cleanup.

## Explicit runtime selection and retained failures

The default game-local `xlive.dll` failed during library loading after settings
were read. The retained debugger capture identifies an access violation within
that module at RVA 319049 while writing `90909090` to unmapped address AA0F5E.
Its file description identifies a game-specific XliveLessNess adaptation.
The debugger also encounters an invalid-handle exception from the module before
that fault; the diagnostic capture handles that debugger exception explicitly
and is not a successful normal-execution result.

Selecting the installed Microsoft XLive DLL alone failed with Win32 error182.
The existing `XLIVE_PRIVATE_RUNTIME.md` explains its missing legacy credential
dependency. The successful run used the already supported `--xlive-dll` and
`--xlive-dependency` options to select the documented, hash-verified Microsoft
XLive 2.0.0673.0 and matching private `msidcrl40.dll`. Four private runtime files
were copied from the retained orchestrator evidence into this packet's ignored
directory; no game or Windows file was replaced. These are real SDK modules,
not fixture service replacements. This result requires the recorded explicit
runtime paths and does not claim the installed replacement DLL now works.

All failed attempts and debugger captures remain available. The first build
recorder invocation was rejected because its environment variable was set too
late; it was not used as acceptance evidence. The correctly configured recorder
and subsequent build passed before the successful run.

This packet closes the startup owner-publication placeholder. It does not
complete the factory Create slot 0071B870, raw resource-manager/cache loading,
parser dependencies, native FH3 dispatch, rendering parity or gameplay. Those
remain separate reconstruction work.
