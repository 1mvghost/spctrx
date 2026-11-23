#ifndef IO_H
#define IO_H

#include <util.h>

void archOut8(u16 port, u8 value);
void archOut16(u16 port, u16 value);
void archOut32(u16 port, u32 value);

u8 archIn8(u16 port);
u16 archIn16(u16 port);
u32 archIn32(u16 port);

#endif