# Original pipe data and CRT registration

Addresses: 00CD6E16, 00CD6E2C, 00CD6E47, 00CD6E62, 00CD6E7D, 00CD6E98, 00CD6F03,
00CE099A, 00CE09A4, 00CE09B5, 00CE09C6, 00CE09D7, 00CE09E8, 00BF6FF5.

This composition supplies the actual original data and real CRT registration
required by XLIVE_PIPE_GLOBALS.md. It does not reconstruct a new native routine.

`XLivePipeOriginalData` opens the caller-selected original executable read-only,
checks its PE32 machine/base and file-backed section bounds, and retains only
the two required ranges: 200980 table bytes at D25D9C and 568 fixed-source bytes
at E12AC8. The existing table and fixed-data classes independently verify their
exact SHA-256 digests before the views are exposed. The original executable is
never loaded as executable code, and the installation is not changed.

`CrtXLivePipeGlobalsStartupHost` creates actual recovered Win32 value locks and
registers the six recovered cleanup actions through real `std::atexit`, returning
the CRT's status unchanged. Concrete no-argument callbacks bind one retained
canonical owner. Address identities are checked against their recovered C++
callbacks; original image addresses are never called. Registrations remain in
the real CRT order, including unrelated callbacks registered between them.
Registration failure does not roll back native construction.

The globals owner must outlive all callbacks and consumers. Construct its static
storage before invoking the initializer sequence so cleanup registrations run
before the owner's C++ storage lifetime ends. An automatic owner that disappears
before process exit violates this contract. No callback is run early by the host
destructor. This supplies the pipe subset's actual registration; unrelated game
CRT initializers and the complete executable startup order remain separate work.

The verified original loader preimage is zero only for these globals' PE virtual
tail. Original protocol heap bytes and framing stack bytes remain separate input
requirements; this loader supplies neither fabricated preimages nor live SDK
initialization. It opens no pipe and starts no worker.

Validation: the strict Win32 build and both existing CTests pass. A focused
isolated process reads the installed image through this loader and passes both
data digests, rejects a relative path, exercises the actual lock factory, and
registers all six callbacks with the real CRT. Five observing wrappers contain
real Win32 sections and confirm reverse destruction order at process exit.
A final callback verifies that order and releases the retained owner only after
the recovered cleanup callbacks. The process exits with code 0. These checks
establish the pipe subset's composition, not complete application startup.
