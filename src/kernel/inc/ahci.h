#ifndef AHCI_H
#define AHCI_H

#include <util.h>

typedef volatile struct {
  u32 clb;
  u32 clbUp;
  u32 fb;
  u32 fbUp;
  u32 is;
  u32 ie;
  u32 cmd;
  u32 reserved;
  u32 tfd;
  u32 sign;
  u32 ssts;
  u32 sCtrl;
  u32 sErr;
  u32 sAct;
  u32 ci;
  u32 sntf;
  u32 fbs;
  u32 reserved1[11];
  u32 vendor[4];
} HbaPort;

bool ahciRead(int p, u64 lba, u32 sectAmount, void* buf);
bool ahciWrite(int p, u64 lba, u32 sectAmount, void* buf);
void ahciInit(u64 bar5);

#endif