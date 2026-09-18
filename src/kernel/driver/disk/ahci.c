#include <ahci.h>
#include <debug.h>
#include <pmm.h>
#include <vmm.h>

#define FIS_TYPE_REG_H2D 0x27

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF 0x20
#define ATA_SR_DSC 0x10
#define ATA_SR_DRQ 0x08
#define ATA_SR_CORR 0x04
#define ATA_SR_IDX 0x02
#define ATA_SR_ERR 0x01

#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000
#define HBA_PxIS_TFES (1 << 30)

typedef volatile struct {
  u32 Cap;
  u32 Ghc;
  u32 IntStatus;
  u32 Pi;
  u32 Version;
  u32 CccCtrl;
  u32 CccPorts;
  u32 EmLoc;
  u32 EmCtrl;
  u32 Cap2;
  u32 Bohc;
  u8 Reserved[0xA0 - 0x2C];
  u8 Vendor[0x100 - 0xA0];
  HbaPort Ports[32];
} HbaMem;

typedef volatile struct {
  u8 CmdFisLen : 5;
  u8 Atapi : 1;
  u8 Write : 1;
  u8 Prefetch : 1;
  u8 Reset : 1;
  u8 Bist : 1;
  u8 ClearBusy : 1;
  u8 Reserved0 : 1;
  u8 PortMulPort : 4;
  u16 PrdtLen;

  volatile u32 Prdbc;

  u32 CtbAddr;
  u32 CtbAddrUp;

  u32 Reserved1[4];
} HbaCmdHeader;

typedef volatile struct {
  u32 DbAddr;
  u32 DbAddrUp;

  u32 Reserved0;

  u32 Dbc : 22;
  u32 Reserved1 : 9;
  u32 I : 1;
} HbaPrdtEnt;

typedef volatile struct {
  u8 CmdFis[64];
  u8 AtapiCmd[16];
  u8 Reserved0[48];
  HbaPrdtEnt Ent[1];
} HbaCmdTbl;

typedef volatile struct {
  u8 FisType;

  u8 PortMult : 4;
  u8 Reserved0 : 3;
  u8 C : 1; /* 1:cmd, 0:control */

  u8 Cmd;
  u8 FeatureLow;

  u8 Lba0;
  u8 Lba1;
  u8 Lba2;
  u8 Device;

  u8 Lba3;
  u8 Lba4;
  u8 Lba5;
  u8 FeatureHigh;

  u8 CountLow;
  u8 CountHigh;
  u8 Icc;
  u8 Control;

  u8 Reserved1[4];
} FisRegHostToDev;

static HbaMem* base = 0;

void ahciCmdStart(HbaPort* port) {
  while (port->Cmd & HBA_PxCMD_CR) {
  }

  port->Cmd |= HBA_PxCMD_ST;
  port->Cmd |= HBA_PxCMD_FRE;
}
void ahciCmdStop(HbaPort* port) {
  port->Cmd &= ~HBA_PxCMD_ST;
  port->Cmd &= ~HBA_PxCMD_FRE;

  while (1) {
    if (port->Cmd & HBA_PxCMD_FR)
      continue;
    if (port->Cmd & HBA_PxCMD_CR)
      continue;
    break;
  }
}
void ahciRebase(HbaPort* port) {
  ahciCmdStop(port);

  /* alloc command list */
  void* cl = vmmAlloc(1);
  memset(cl, 0, PAGE_SIZE);

  u64 clPhys = vmmVirtToPhys(cl);
  port->Clb = U64_LOW(clPhys);
  port->ClbUp = U64_HIGH(clPhys);

  /* alloc fis */
  void* fis = vmmAlloc(1);
  memset(fis, 0, PAGE_SIZE);

  u64 fisPhys = vmmVirtToPhys(fis);
  port->Fb = U64_LOW(fisPhys);
  port->FbUp = U64_HIGH(fisPhys);

  /* alloc command table */
  HbaCmdHeader* cmd = (HbaCmdHeader*)cl;

  for (int i = 0; i < 32; i++) {
    cmd[i].PrdtLen = 8; /* 8 ENTRIES PER CMD TABLE */

    void* ctba = vmmAlloc(1);
    memset(ctba, 0, PAGE_SIZE);

    u64 ctbaPhys = vmmVirtToPhys(ctba);
    cmd[i].CtbAddr = U64_LOW(ctbaPhys);
    cmd[i].CtbAddrUp = U64_HIGH(ctbaPhys);
  }

  ahciCmdStart(port);
}

int ahciCmdFindFree(HbaPort* port) {
  u32 slots = (port->SAct | port->Ci);
  u32 cmdSlots = (base->Cap & 0x0f00) >> 8;

  for (u32 i = 0; i < cmdSlots; i++) {
    if (!(slots & 1)) {
      return i;
    }
    slots >>= 1;
  }
  debug("ahci: CANNOT FIND FREE CMD SLOT\n");
  return -1;
}
int ahciCheckType(HbaPort* port) {
  u8 ipm = (port->Ssts >> 8) & 0x0F;
  u8 det = port->Ssts & 0x0F;

  if (det != HBA_PORT_DET_PRESENT) {
    return 0;
  }
  if (ipm != HBA_PORT_IPM_ACTIVE) {
    return 0;
  }
  switch (port->Sign) {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  }
}

bool ahciSendCommand(HbaPort* port) {
  int wait = 1000000000;
  int spin = 0;

  while (port->Tfd & (ATA_SR_BSY | ATA_SR_DRQ) && spin < wait) {
    spin++;
  }
  if (spin == wait) {
    debug("ahci: PORT HUNG\n");
    return false;
  }

  port->Ci = 1;

  while (port->Ci == 1 && !(port->Is & HBA_PxIS_TFES)) {
  }

  if (port->Is & HBA_PxIS_TFES) {
    debug("ahci: ERROR\n");
    return false;
  }

  return true;
}

bool ahciReadOrWrite(int p, u64 lba, u32 sectAmount, void* buf, bool write) {
  HbaPort* port = &base->Ports[p];
  port->Is = 0xFFFFFFFF;

  int slot = ahciCmdFindFree(port);
  if (slot == -1) {
    debug("ahci: no free slots right now\n");
    return false;
  }

  HbaCmdHeader* cmd = (HbaCmdHeader*)vmmPhysToVirt(U64(port->Clb, port->ClbUp));
  if (!cmd)
    return false;

  cmd[slot].CmdFisLen = sizeof(FisRegHostToDev) / sizeof(u32);
  cmd[slot].Write = write;

  HbaCmdTbl* table =
      (HbaCmdTbl*)vmmPhysToVirt(U64(cmd[slot].CtbAddr, cmd[slot].CtbAddrUp));
  if (!table)
    return false;

  u64 bufPhys = vmmVirtToPhys(buf);

  table->Ent[0].DbAddr = U64_LOW(bufPhys);
  table->Ent[0].DbAddrUp = U64_HIGH(bufPhys);
  table->Ent[0].Dbc = (512 * sectAmount) - 1;
  table->Ent[0].I = 1;

  FisRegHostToDev* fis = (FisRegHostToDev*)&table->CmdFis;

  fis->FisType = FIS_TYPE_REG_H2D;
  fis->C = 1;
  fis->Cmd = ATA_CMD_READ_DMA_EXT;

  if (write)
    fis->Cmd = ATA_CMD_WRITE_DMA_EXT;

  fis->Lba0 = lba & 0xFF;
  fis->Lba1 = (lba >> 8) & 0xFF;
  fis->Lba2 = (lba >> 16) & 0xFF;

  fis->Device = 64;

  fis->Lba3 = (lba >> 24) & 0xFF;
  fis->Lba4 = (lba >> 32) & 0xFF;
  fis->Lba5 = (lba >> 40) & 0xFF;

  fis->CountLow = sectAmount & 0xFF;
  fis->CountHigh = (sectAmount >> 8) & 0xFF;

  return ahciSendCommand(port);
}

bool ahciRead(int p, u64 lba, u32 sectAmount, void* buf) {
  return ahciReadOrWrite(p, lba, sectAmount, buf, false);
}
bool ahciWrite(int p, u64 lba, u32 sectAmount, void* buf) {
  return ahciReadOrWrite(p, lba, sectAmount, buf, true);
}

void ahciEnum() {
  u32 pi = base->Pi;
  int i;
  for (i = 0; i < 32; i++) {
    if (pi & 1) {
      int type = ahciCheckType(&base->Ports[i]);
      if (type == AHCI_DEV_SATA) {
        debug("ahci: SATA drive found at port %d\n", i);
        ahciRebase(&base->Ports[i]);
      } else if (type == AHCI_DEV_SATAPI) {
        debug("ahci: SATAPI drive found at port %d\n", i);
      } else if (type == AHCI_DEV_SEMB) {
        debug("ahci: SEMB drive found at port %d\n", i);
      } else if (type == AHCI_DEV_PM) {
        debug("ahci: PM drive found at port %d\n", i);
      }
    }
    pi >>= 1;
  }
}
void ahciInit(u64 bar5) {
  if (base != 0) {
    debug("ahci: more than 1 controller is not supported yet\n");
    return;
  }
  vmmMap(vmmPhysToVirt(bar5), bar5, PTE_WRITABLE);
  base = (HbaMem*)vmmPhysToVirt(bar5);
  ahciEnum();
}