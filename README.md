# k11_dumper
Kernel11 live memory dumper

**Works on vanilla kernel.**

Does it job by adding access to the `CONFIG11` page to its ExHeader (see the `rsf` file) then sets `CONFIG11_GPUPROT` to 0 so that the GPU DMA can access the full contents of FCRAM, AXIWRAM, and QTM/N3DS extra mem.
