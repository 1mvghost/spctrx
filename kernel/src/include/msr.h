#ifndef MSR_H
#define MSR_H

#include <util.h>

bool msrCheckSupport();

void wrmsr(u32 msr, u64 value);
u64 rdmsr(u32 msr);

#endif