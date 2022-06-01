---
layout: page
title: "WebAssembly paper"
permalink: /paper
---
# Not So Fast: Analyzing the Performance of WebAssembly vs. Native Code

## Introduction

[This paper](https://www.usenix.org/conference/atc19/presentation/jangda) discusses the performance of code compiled to WebAssembly in a browser compared to code that is compiled using the Clang compiler to run natively. This comparison has previously only been made for scientific benchmarks like PolyBenchC where the results show only a small performance decrease in speed (less than 1.1 in most cases). However, these benchmarks are not representative of larger applications and therefore a benchmark set like SPEC CPU is more appropriate. These benchmarks do however require WebAssembly to make system calls, something that is not natively supported and did require a workaround.

## Benchmarking WebAssembly

As mentioned before the previous benchmarks were all scientific benchmarks. These benchmarks mainly rely on matrix operations consisting of about 100 lines of code. Although one of the use cases of WebAssembly might be accelerating scientific calculations on the web, it certainly is not what most people will use it for.

## Executing SPEC CPU in WebAssembly.

An alternative benchmarking tool is SPEC CPU. It has a lot of different benchmarks in a wide variety of application areas. A few of them include:

- A Perl interpreter,
- A GNU C compiler,
- AI tests,
- Data compression,
- Ray tracing,
- 3D rendering.

This sounds great but it comes with a few drawbacks. As it is intended to be run as an application it relies on certain features like a filesystem and the ability to spawn processes. When you want to run your own program in WebAssembly you can work around these issues but doing that with a benchmark would invalidate the results.

It is clear that we need a way to run UNIX applications natively in WebAssembly. Such frameworks already exist. One of them is BROWSIX. It implements a UNIX-compatible kernel in JavaScript with support for all the features we were missing earlier. The only downside is that BROWSIX was made before WebAssembly existed and only supports JavaScript.

BROWSIX can however be modified to work with WebAssembly. BROWSIX does have a significant memory and performance impact due to the way it manages expanding and allocating memory. This would make the results of the benchmarks that are being explored unreliable. This can be improved however by allocating only a small initial auxiliary array buffer and expanding it in chunks instead of reallocating when the size increases. This augmented version of BROWSIX can be called BROWSIX-WASM and by further including performance measuring it becomes the BROWSIX-SPEC that is used in these benchmarks.


## Results 

In the PolyBenhC benchmark-set the performance is the same as it has been stated in previous works. This proves the BROWSIX method does not introduce a noticeable performance impact that would devalue the other results. These other results however show a problem in WebAssembly. While WebAssembly is still measurably faster (about 1.54x) than the fastest JavaScript (asm.js) code there is a steep impact that was not seen before when compared to natively compiled code in
SPEC-CPU (1.55x slower on average). One note is that this only measures execution time. The native code took a lot longer to actually compile then its WebAssembly equivalent. This is especially important as WebAssembly is usually compiled when a website loads.

## Reasons

### More loads and stores

When comparing the native and WebAssembly code we can see that WebAssembly usually needs more than double the amount of load and stores in comparison with native code. This is because WebAssembly does not use all of the x86 memory addressing modes. An example would be the add instruction with register addressing mode. Native code loads and writes to the registers in one operation. WebAssembly on the other hand first loads the address, performs the addition and then stores the result requiring three instructions instead of one.

### WASM Compilers vs CLANG.

WASM compilers work faster than CLANG but will produce less optimised code. It produces more instructions, and this leads to more cache misses. Cache misses make the CPU wait while data is being fetched in memory. 

### Register pressure.

WASM does not use all the possible register addressing methods resulting in sub optimal usage of the register combined with losing two registers to the structure of the browser this creates pressure on the register which results in spills to memory.

### Safety checks.

WASM needs to do extra safety checks in runtime on every jump to see if it jumps to a valid index this also generates extra CPU cycles.

## Discussion

The results found in this paper show that further compiler optimisations might still be necessary for WebAssembly. This however will not eliminate the gap completely because a substantial portion of the slowdown is inherent to the architecture of the language itself. Furthermore, we do think that while WebAssembly did need to be evaluated in a wider set of benchmarks that better represent real world use cases, SPEC C might not have been the correct choice due to the extensive use of system calls, something WebAssembly was never designed to do. To reduce the register pressure a version of WebAssembly that is not designed to run in conjunction with JavaScript might be an option because then there would be no need for two registers for the JavaScript garbage collector. This however would mean that the current structure of a WASM backend and a JS frontend would no longer be possible which is not a good trade-off.

