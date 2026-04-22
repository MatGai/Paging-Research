# Final Year Dissertation Project — Page Sizes on Modern Systems 

The scope of this project is to explore how page size choices influence hardware behaviour. The objective is to build an interface that maps an executable while allowing control over page sizes (4 KiB, 2 MiB, and 1 GiB) across different memory regions and mappings (uniform vs mixed).

Using these mappings, I run microbenchmarks designed to stress parts of the MMU (Memory Management Unit), particularly the ITLB and DTLB. Hardware performance counters (PMU) are used to measure events such as ITLB/DTLB misses, page walks, CPU cycles, and instructions retired.

The aim of the project is to produce concrete measurements showing how these design choices can affect hardware behaviour and workload performance.

<h3>Unified Extensible Firmware Interface (UEFI)</h3>

UEFI was chosen as the environment for this project because it is close to the hardware and provides a clean environment to work in. This helps reduce interference in measurements and makes the results easier to interpret.

It was chosen over legacy BOOT because of its usability and 'extensibility' ;) . In practice, it meant I could boot straight into an environment where alot was setup, such as file access and graphics output, which let me spend more time on the actual research rather than rebuilding basic infrastructure.


<p>To add: this is my first somewhat large project. Expect some spaghetti and interesting design choices.</p>
