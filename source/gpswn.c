#include "gspwn.h"

// Source: https://github.com/smealum/udsploit/blob/master/source/kernel.c#L11
static Result gspSetTextureCopyPhys(u32 in_pa, u32 out_pa, u32 size, u32 in_dim, u32 out_dim, u32 flags)
{
    u32 enable_reg = 0;
    Result ret = 0;

    ret = GSPGPU_ReadHWRegs(0x1EF00C18 - 0x1EB00000, &enable_reg, sizeof(enable_reg));
    if(ret) return ret;

    ret = GSPGPU_WriteHWRegs(0x1EF00C00 - 0x1EB00000, (u32[]){in_pa >> 3, out_pa >> 3}, 0x8);
    if(ret) return ret;
    ret = GSPGPU_WriteHWRegs(0x1EF00C20 - 0x1EB00000, (u32[]){size, in_dim, out_dim}, 0xC);
    if(ret) return ret;
    ret = GSPGPU_WriteHWRegs(0x1EF00C10 - 0x1EB00000, &flags, 4);
    if(ret) return ret;
    ret = GSPGPU_WriteHWRegs(0x1EF00C18 - 0x1EB00000, (u32[]){enable_reg | 1}, 4);
    if(ret) return ret;

    svcSleepThread(50 * 1000 * 1000LL); // should be enough
    return 0;
}

Result gspwn(u32 dstPa, u32 srcPa, u32 size)
{
    return gspSetTextureCopyPhys(srcPa, dstPa, size, 0, 0, 8);
}
