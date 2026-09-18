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
  u32 cap;
  u32 ghc;
  u32 intStatus;
  u32 pi;
  u32 version;
  u32 cccCtrl;
  u32 cccPorts;
  u32 emLoc;
  u32 emCtrl;
  u32 cap2;
  u32 bohc;
  u8 reserved[0xA0 - 0x2C];
  u8 vendor[0x100 - 0xA0];
  HbaPort ports[32];
} HbaMem;

typedef volatile struct {
  u8 cmdFisLen : 5;
  u8 atapi : 1;
  u8 write : 1;
  u8 prefetch : 1;
  u8 reset : 1;
  u8 bist : 1;
  u8 clearBusy : 1;
  u8 reserved0 : 1;
  u8 portMulPort : 4;
  u16 prdtLen;

  volatile u32 prdbc;

  u32 ctbAddr;
  u32 ctbAddrUp;

  u32 reserved1[4];
} HbaCmdHeader;

typedef volatile struct {
  u32 dbAddr;
  u32 dbAddrUp;

  u32 reserved0;

  u32 dbc : 22;
  u32 reserved1 : 9;
  u32 i : 1;
} HbaPrdtEnt;

typedef volatile struct {
  u8 cmdFis[64];
  u8 atapiCmd[16];
  u8 reserved0[48];
  HbaPrdtEnt ent[1];
} HbaCmdTbl;

typedef volatile struct {
  u8 fisType;

  u8 portMult : 4;
  u8 reserved0 : 3;
  u8 c : 1; /* 1:cmd, 0:control */

  u8 cmd;
  u8 featureLow;

  u8 lba0;
  u8 lba1;
  u8 lba2;
  u8 device;

  u8 lba3;
  u8 lba4;
  u8 lba5;
  u8 featureHigh;

  u8 countLow;
  u8 countHigh;
  u8 icc;
  u8 control;

  u8 reserved1[4];
} FisRegHostToDev;

static HbaMem* base = 0;

void ahciCmdStart(HbaPort* port) {
  while (port->cmd & HBA_PxCMD_CR) {
  }

  port->cmd |= HBA_PxCMD_ST;
  port->cmd |= HBA_PxCMD_FRE;
}
void ahciCmdStop(HbaPort* port) {
  port->cmd &= ~HBA_PxCMD_ST;
  port->cmd &= ~HBA_PxCMD_FRE;

  while (1) {
    if (port->cmd & HBA_PxCMD_FR)
      continue;
    if (port->cmd & HBA_PxCMD_CR)
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
  port->clb = U64_LOW(clPhys);
  port->clbUp = U64_HIGH(clPhys);

  /* alloc fis */
  void* fis = vmmAlloc(1);
  memset(fis, 0, PAGE_SIZE);

  u64 fisPhys = vmmVirtToPhys(fis);
  port->fb = U64_LOW(fisPhys);
  port->fbUp = U64_HIGH(fisPhys);

  /* alloc command table */
  HbaCmdHeader* cmd = (HbaCmdHeader*)cl;

  for (int i = 0; i < 32; i++) {
    cmd[i].prdtLen = 8; /* 8 ENTRIES PER CMD TABLE */

    void* ctba = vmmAlloc(1);
    memset(ctba, 0, PAGE_SIZE);

    u64 ctbaPhys = vmmVirtToPhys(ctba);
    cmd[i].ctbAddr = U64_LOW(ctbaPhys);
    cmd[i].ctbAddrUp = U64_HIGH(ctbaPhys);
  }

  ahciCmdStart(port);
}

int ahciCmdFindFree(HbaPort* port) {
  u32 slots = (port->sAct | port->ci);
  u32 cmdSlots = (base->cap & 0x0f00) >> 8;

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
  u8 ipm = (port->ssts >> 8) & 0x0F;
  u8 det = port->ssts & 0x0F;

  if (det != HBA_PORT_DET_PRESENT) {
    return 0;
  }
  if (ipm != HBA_PORT_IPM_ACTIVE) {
    return 0;
  }
  switch (port->sign) {
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

  while (port->tfd & (ATA_SR_BSY | ATA_SR_DRQ) && spin < wait) {
    spin++;
  }
  if (spin == wait) {
    debug("ahci: PORT HUNG\n");
    return false;
  }

  port->ci = 1;

  while (port->ci == 1 && !(port->is & HBA_PxIS_TFES)) {
  }

  if (port->is & HBA_PxIS_TFES) {
    debug("ahci: ERROR\n");
    return false;
  }

  return true;
}

bool ahciReadOrWrite(int p, u64 lba, u32 sectAmount, void* buf, bool write) {
  HbaPort* port = &base->ports[p];
  port->is = 0xFFFFFFFF;

  int slot = ahciCmdFindFree(port);
  if (slot == -1) {
    debug("ahci: no free slots right now\n");
    return false;
  }

  HbaCmdHeader* cmd = (HbaCmdHeader*)vmmPhysToVirt(U64(port->clb, port->clbUp));
  if (!cmd)
    return false;

  cmd[slot].cmdFisLen = sizeof(FisRegHostToDev) / sizeof(u32);
  cmd[slot].write = write;

  HbaCmdTbl* table =
      (HbaCmdTbl*)vmmPhysToVirt(U64(cmd[slot].ctbAddr, cmd[slot].ctbAddrUp));
  if (!table)
    return false;

  u64 bufPhys = vmmVirtToPhys(buf);

  table->ent[0].dbAddr = U64_LOW(bufPhys);
  table->ent[0].dbAddrUp = U64_HIGH(bufPhys);
  table->ent[0].dbc = (512 * sectAmount) - 1;
  table->ent[0].i = 1;

  FisRegHostToDev* fis = (FisRegHostToDev*)&table->cmdFis;

  fis->fisType = FIS_TYPE_REG_H2D;
  fis->c = 1;
  fis->cmd = ATA_CMD_READ_DMA_EXT;

  if (write)
    fis->cmd = ATA_CMD_WRITE_DMA_EXT;

  fis->lba0 = lba & 0xFF;
  fis->lba1 = (lba >> 8) & 0xFF;
  fis->lba2 = (lba >> 16) & 0xFF;

  fis->device = 64;

  fis->lba3 = (lba >> 24) & 0xFF;
  fis->lba4 = (lba >> 32) & 0xFF;
  fis->lba5 = (lba >> 40) & 0xFF;

  fis->countLow = sectAmount & 0xFF;
  fis->countHigh = (sectAmount >> 8) & 0xFF;

  return ahciSendCommand(port);
}

bool ahciRead(int p, u64 lba, u32 sectAmount, void* buf) {
  return ahciReadOrWrite(p, lba, sectAmount, buf, false);
}
bool ahciWrite(int p, u64 lba, u32 sectAmount, void* buf) {
  return ahciReadOrWrite(p, lba, sectAmount, buf, true);
}

void ahciEnum() {
  u32 pi = base->pi;
  int i;
  for (i = 0; i < 32; i++) {
    if (pi & 1) {
      int type = ahciCheckType(&base->ports[i]);
      if (type == AHCI_DEV_SATA) {
        debug("ahci: SATA drive found at port %d\n", i);
        ahciRebase(&base->ports[i]);
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