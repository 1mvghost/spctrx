#ifndef ARCH_H
#define ARCH_H

void archDisableInterrupts();
void archEnableInterrupts();
void archWaitForInterrupt();
void archHcf();

void archDebugPutc(char ch);

void archEarlyInit();
void archApEarlyInit();

#endif