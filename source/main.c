#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "gspwn.h"

#define CONFIG11_BASE   0x1EC40000
#define TRY(expr)       if (R_FAILED((res = expr))) return res;
#define IS_N3DS         (*(vu32 *)0x1FF80030 >= 6) // APPMEMTYPE. Hacky but doesn't use APT

u32 __ctru_linear_heap_size = 48 << 20;
u32 __ctru_heap_size        = 10 << 20;

#define CHUNK_SIZE      (8 << 20)

static Result dumpMemRegion(void *workbuf, void *filebuf, u32 pa, u32 size, const char *filename)
{
    Result res = 0;
    u32 workbufPa = osConvertVirtToPhys(workbuf);

    printf("Dumping %s\n", filename);

    // Flush everything with this one weird trick
    TRY(GSPGPU_FlushDataCache((void *)0x14000000, 0x700010));

    TRY(gspwn(workbufPa, pa, size));

    // Flush everything with this one weird trick
    TRY(GSPGPU_FlushDataCache((void *)0x14000000, 0x700010));

    FILE *f = fopen(filename, "wb+");
    if (f == NULL) {
        return -1;
    }

    u32 remaining = size;
    u32 n = 0;
    while (remaining > 0) {
        u32 sz = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;
        memcpy(filebuf, (u8 *)workbuf + n * CHUNK_SIZE, sz);
        fwrite(filebuf, 1, sz, f);
        n++;
        remaining -= sz;

        printf("Wrote chunk %lu\n", n);
    }
    fclose(f);

    printf("Dumped %s\n", filename);

    return res;
}

static Result dumpKernelMemEtc(void *workbuf, void *filebuf)
{
    Result res = 0;

    // Set CONFIG11_GPUPROT to 0
    *(vu32 *)(CONFIG11_BASE + 0x140) = 0;

    TRY(dumpMemRegion(workbuf, filebuf, 0x1FF80000, 0x80000, "axiwram.bin"));
    if (IS_N3DS) {
        TRY(dumpMemRegion(workbuf, filebuf, 0x1F000000, 0x400000, "n3ds_extramem.bin"));
        TRY(dumpMemRegion(workbuf, filebuf, 0x2E000000, 0x2000000, "base_region.bin"));
    } else {
        TRY(dumpMemRegion(workbuf, filebuf, 0x26C00000, 0x1400000, "base_region.bin"));
    }

    return res;
}

int main(int argc, char* argv[])
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    void *workbuf = linearAlloc(0x2000000);
    void *filebuf = malloc(CHUNK_SIZE);
    if (workbuf != NULL && filebuf != NULL) {
        printf("Attempting to dump kernel regions...\n");
        printf("Done, result = %08lx\n", (u32)dumpKernelMemEtc(workbuf, filebuf));
    } else {
        printf("Failed to allocate: workbuf=%p filebuf=%p\n", workbuf, filebuf);
    }

    // Main loop
    while (aptMainLoop()) {
        gspWaitForVBlank();
        gfxSwapBuffers();
        hidScanInput();

        // Your code goes here
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }
    }

    free(filebuf);
    linearFree(workbuf);
    gfxExit();
    return 0;
}
