// Restore ONLY the independently byte-verified oct18-tA receive-factory body.
// Invoke under coordination.ghidra_lock; keep the typed preimage and full function
// documentation receipt before running. No instruction/flow/prototype/comment edits.
// @category BSP
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.Function;
import java.security.MessageDigest;
import ghidra.program.model.address.Address;

public class RepairOct18FactoryBody extends GhidraScript {
    public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname().equals("/battlestationspacific.exe"))
            throw new IllegalStateException("Wrong program");
        byte[] bytes = new byte[0x1d68];
        if (currentProgram.getMemory().getBytes(toAddr(0x00768530L), bytes) != bytes.length)
            throw new IllegalStateException("Incomplete native bytes");
        StringBuilder hash = new StringBuilder();
        for (byte b : MessageDigest.getInstance("SHA-256").digest(bytes))
            hash.append(String.format("%02x", b & 255));
        if (!hash.toString().equals("f026559defeeea77eae2c8e5b1f1936c1dbc2075329423e62d73f5634266e7c4"))
            throw new IllegalStateException("Saved image does not match reviewed disk bytes");
        Function function = currentProgram.getFunctionManager().getFunctionAt(toAddr(0x00768530L));
        if (function == null || !function.getName().equals("BSP_SessionMessage_CreateFromBitStream"))
            throw new IllegalStateException("Function identity changed");
        AddressSet wanted = new AddressSet(toAddr(0x00768530L), toAddr(0x0076a297L));
        if (function.getBody().equals(wanted)) {
            println("00768530: verified complete body already installed");
            return;
        }
        AddressSet previous = new AddressSet(toAddr(0x00768530L), toAddr(0x0076857fL));
        previous.add(toAddr(0x0076a277L), toAddr(0x0076a297L));
        if (!function.getBody().equals(previous))
            throw new IllegalStateException("Body preimage changed; recapture before any write");
        for (Address address = wanted.getMinAddress();
             address.compareTo(wanted.getMaxAddress()) <= 0; address = address.add(1)) {
            Function owner = currentProgram.getFunctionManager().getFunctionContaining(address);
            if (owner != null && !owner.getEntryPoint().equals(function.getEntryPoint()))
                throw new IllegalStateException("Another function owns part of the reviewed span");
        }
        int transaction = currentProgram.startTransaction("oct18-tA verified receive-factory body");
        boolean commit = false;
        try {
            function.setBody(wanted);
            if (!function.getBody().equals(wanted)) throw new IllegalStateException("Body readback mismatch");
            commit = true;
        } finally {
            currentProgram.endTransaction(transaction, commit);
        }
        println("00768530: restored 00768530..0076a297, preserving all listing, flow, names and comments");
    }
}
