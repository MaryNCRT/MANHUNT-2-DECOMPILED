// Ghidra headless script: prints the call tree below a function.
//
// Each line: depth indent, address, name, body size, and the memory block the
// function lives in (so calls into the .secu leftover stand out).
//
//   analyzeHeadless OUTPUT\ghidra MH2 -process Manhunt2_nosecu.exe -noanalysis -readOnly \
//       -scriptPath OUTPUT\tools -postScript CallTree.java <0xADDR> <depth> <out.txt>
//
//@category MH2

import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;
import java.util.TreeSet;
import java.util.Comparator;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.MemoryBlock;

public class CallTree extends GhidraScript {

    private PrintWriter out;
    private final Set<Address> seen = new HashSet<>();

    private void walk(Function f, int depth, int max) throws Exception {
        MemoryBlock b = currentProgram.getMemory().getBlock(f.getEntryPoint());
        boolean again = !seen.add(f.getEntryPoint());
        out.println("  ".repeat(depth) + f.getEntryPoint() + " " + f.getName() + " ("
                + f.getBody().getNumAddresses() + " B, " + (b == null ? "?" : b.getName()) + ")"
                + (again ? " ..." : ""));
        if (again || depth >= max) {
            return;
        }
        Set<Function> callees = new TreeSet<>(Comparator.comparing(Function::getEntryPoint));
        callees.addAll(f.getCalledFunctions(monitor));
        for (Function c : callees) {
            walk(c, depth + 1, max);
        }
    }

    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        Address addr = toAddr(a[0]);
        Function f = getFunctionContaining(addr);
        try (PrintWriter w = new PrintWriter(a[2], "UTF-8")) {
            out = w;
            walk(f, 0, Integer.parseInt(a[1]));
        }
        println("wrote " + a[2]);
    }
}
