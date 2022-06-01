---
layout: page
title: WebAssembly paper: Not So Fast: Analyzing the Performance
of WebAssembly vs. Native Code

permalink: /paper
---
##Introduction

This paper discusses the performance of code compiled to web assembly in a browser compared to code that is compiled using the Clang compiler to run natively. This comparison has previously only been made for scientific benchmarks like PolyBenchC where the results show only a small performance decrease in speed (less than 1.1 in most cases) . However these benchmarks are not representative of larger applications and therefore a benchmark set like SPEC CPU is more appropriate. These benchmarks do however require web assembly to make system calls something that is not natively supported and did require a workaround.
Executing SPEC CPU in web assembly.

This task can be achieved by modifying BROWSIX to work with web assembly. BROWSIX does however have a significant memory and performance impact due to the way it handles expanding and allocating memory. This would make the results of the benchmarks that are being explored unreliable. But this can be ameliorated by allocating only a small initial auxiliary array buffer and expanding it in chunks instead of reallocating when the size increases. This augmented version of BROWSIX can be called BROWSIX-WASM and by further including performance measuring it becomes the BROWSIX-SPEC that is used in these benchmarks.

##Results 
In the PolyBenhC benchmark set the performance is the same as it has been stated in previous works. This proves the BROWSIX method does not introduce a noticeable performance impact that would devalue the other results. These other results however show a problem in web assembly. While web assembly is still measurably faster(about 1.54x) than the fastest JavaScript (asm.js) code there is a steep impact that wasn’t seen before when compared to natively compiled code in
SPEC-CPU(1.55x slower on average).

##Reasons
###WASM Compilers vs CLANG.
WASM compilers work faster than CLANG but will produce less optimised code. It produces more instructions and this leads to more cache misses. Cache misses make the CPU wait while data is being fetched in memory. 
###Register pressure.
WASM does not use all of the possible register addressing methods resulting in sub optimal usage of the register combined with losing 2 registers to the structure of the browser this creates pressure on the register which results in spills to memory.
###Safety checks.
WASM needs to do extra safety checks in runtime on every jump to see if it jumps to a valid index this also generates extra CPU cycles.
#Discussion
The results found in this paper show that further compiler optimisations might still be necessary for web assembly. This however won’t eliminate the gap completely because a big portion of the slowdown is inherent to the architecture of the language itself. Furthermore do we think that while web assembly did need to be tested in a wider set of benchmarks that better represent real world use cases, SPEC C might not have been the correct choice due to the extensive use of system calls something web assembly was never designed to do. To reduce the register pressure a version of web assembly that isn’t designed to run in conjunction with JavaScript might be an option because then there wont be a need for 2 registers for the JavaScript garbage collector. This however would mean that the current structure of a WASM backend and a JS frontend would no longer be possible which in our opinion isn’t a good trade-off.

