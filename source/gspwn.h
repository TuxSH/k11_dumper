#pragma once

#include <3ds/svc.h>
#include <3ds/result.h>
#include <3ds/services/gspgpu.h>

Result gspwn(u32 dstPa, u32 srcPa, u32 size);
