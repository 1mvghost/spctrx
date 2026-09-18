#include <debug.h>
#include <ide.h>
#include <vmm.h>

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07
#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0A
#define ATA_REG_LBA5 0x0B
#define ATA_REG_CONTROL 0x0C
#define ATA_REG_ALTSTATUS 0x0C
#define ATA_REG_DEVADDRESS 0x0D

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

#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLINDERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

#define ATA_READ 0x00
#define ATA_WRITE 0x01

struct {
  u16 Base;
  u16 Ctrl;
  u16 BMIde;
  u8 NieN;
} channels[2];

struct {
  u8 Reserved;
  u8 Channel;
  u8 Drive;
  u16 Type;
  u16 Sign;
  u16 Capabilities;
  u32 CmdSets;
  u32 Size;
  u8 Model[41];
} dev[4];

/**
 * TODO: cleanup
 */

static u8 buf[2048];
static volatile bool irqInvoked = 0;

void ideSleep() {
  for (int i = 0; i < 4; i++)
    in8(ATA_REG_ALTSTATUS);
}
u8 ideIn8(u8 ch, u8 reg) {
  u8 res;
  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, 0x80 | channels[ch].NieN);
  }
  if (reg < 0x08)
    res = in8(channels[ch].Base + reg - (u8)0x00);
  else if (reg < 0x0C)
    res = in8(channels[ch].Base + reg - (u8)0x06);
  else if (reg < 0x0E)
    res = in8(channels[ch].Ctrl + reg - (u8)0x0C);
  else if (reg < 0x16)
    res = in8(channels[ch].BMIde + reg - (u8)0x0E);

  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, channels[ch].NieN);
  }
  return res;
}
void ideWaitBsy(u8 ch) {
  while (ideIn8(ch, ATA_REG_STATUS) & ATA_SR_BSY)
    ;
}
void ideOut8(u8 ch, u8 reg, u8 data) {
  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, 0x80 | channels[ch].NieN);
  }
  if (reg < 0x08)
    out8(channels[ch].Base + reg - 0x00, data);
  else if (reg < 0x0C)
    out8(channels[ch].Base + reg - 0x06, data);
  else if (reg < 0x0E)
    out8(channels[ch].Ctrl + reg - 0x0C, data);
  else if (reg < 0x16)
    out8(channels[ch].BMIde + reg - 0x0E, data);

  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, channels[ch].NieN);
  }
}
void ideInBuf(u8 ch, u8 reg, u32* buf, u64 q) {
  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, 0x80 | channels[ch].NieN);
  }
  asm("pushw %bx; movw %es, %bx; pushw %ax; movw %ds, %ax; movw %ax, %es; popw "
      "%ax;");
  if (reg < 0x08)
    ins32(channels[ch].Base + reg - 0x00, buf, q);
  else if (reg < 0x0C)
    ins32(channels[ch].Base + reg - 0x06, buf, q);
  else if (reg < 0x0E)
    ins32(channels[ch].Ctrl + reg - 0x0C, buf, q);
  else if (reg < 0x16)
    ins32(channels[ch].BMIde + reg - 0x0E, buf, q);
  asm("movw %bx, %es; popw %bx;");
  if (reg > 0x07 && reg < 0x0C) {
    ideOut8(ch, ATA_REG_CONTROL, channels[ch].NieN);
  }
}

u8 idePoll(u8 ch, bool adv) {
  ideSleep();
  ideWaitBsy(ch);

  if (adv) {
    u8 state = ideIn8(ch, ATA_REG_STATUS);

    if (state & ATA_SR_ERR)
      return 2;
    if (state & ATA_SR_DF)
      return 1;

    if (!(state & ATA_SR_DRQ))
      return 3;
  }
  return 0;
}

u8 ideAccessAta(u8 dir, u8 disk, u32 lba, u8 sectAmount, u16* buf) {
  u8 lbaMode = 0; /* 0:chs, 1:lba28, 2:lba48 */
  bool dma = 0;
  u8 cmd = 0;

  u8 lbaIo[6];
  memset(lbaIo, 0, sizeof(lbaIo));

  u8 ch = (u8)dev[disk].Channel;
  u8 slave = (u8)dev[disk].Drive;
  u16 bus = channels[ch].Base;

  u32 words = 256;
  u16 i = 0;
  u8 head = 0;

  ideOut8(ch, ATA_REG_CONTROL, channels[ch].NieN = (irqInvoked = 0x0) + 0x02);

  u16 lbaSupport = dev[disk].Capabilities & 0x200;
  if (lba >= 0x10000000) {
    lbaMode = 2;
    lbaIo[0] = (lba & 0x000000FF) >> 0;
    lbaIo[1] = (lba & 0x0000FF00) >> 8;
    lbaIo[2] = (lba & 0x00FF0000) >> 16;
    lbaIo[3] = (lba & 0xFF000000) >> 24;
    lbaIo[4] = 0;
    lbaIo[5] = 0;
    head = 0;
  } else if (lbaSupport) {
    lbaMode = 1;
    lbaIo[0] = (lba & 0x00000FF) >> 0;
    lbaIo[1] = (lba & 0x000FF00) >> 8;
    lbaIo[2] = (lba & 0x0FF0000) >> 16;
    lbaIo[3] = 0;
    lbaIo[4] = 0;
    lbaIo[5] = 0;
    head = (lba & 0xF000000) >> 24;
  } else {
    u16 cyl = 0;
    u16 sect = 0;
    lbaMode = 0;
    sect = (lba % 63) + 1;
    cyl = (lba + 1 - sect) / (16 * 63);
    lbaIo[0] = sect;
    lbaIo[1] = (cyl >> 0) & 0xFF;
    lbaIo[2] = (cyl >> 8) & 0xFF;
    lbaIo[3] = 0;
    lbaIo[4] = 0;
    lbaIo[5] = 0;
    head = (lba + 1 - sect) % (16 * 63) / (63);
  }

  ideWaitBsy(ch);

  if (lbaMode == 0) {
    ideOut8(ch, ATA_REG_HDDEVSEL, (u8)(0xA0 | (slave << 4) | head));
  } else {
    ideOut8(ch, ATA_REG_HDDEVSEL, (u8)(0xE0 | (slave << 4) | head));
  }

  /* WRITE PARAMETERS */
  if (lbaMode == 2) {
    ideOut8(ch, ATA_REG_SECCOUNT1, 0);
    ideOut8(ch, ATA_REG_LBA3, lbaIo[3]);
    ideOut8(ch, ATA_REG_LBA4, lbaIo[4]);
    ideOut8(ch, ATA_REG_LBA5, lbaIo[5]);
  }
  ideOut8(ch, ATA_REG_SECCOUNT0, (u8)sectAmount);
  ideOut8(ch, ATA_REG_LBA0, lbaIo[0]);
  ideOut8(ch, ATA_REG_LBA1, lbaIo[1]);
  ideOut8(ch, ATA_REG_LBA2, lbaIo[2]);

  if (lbaMode == 0 && dma == 0 && dir == 0)
    cmd = ATA_CMD_READ_PIO;
  if (lbaMode == 1 && dma == 0 && dir == 0)
    cmd = ATA_CMD_READ_PIO;
  if (lbaMode == 2 && dma == 0 && dir == 0)
    cmd = ATA_CMD_READ_PIO_EXT;
  if (lbaMode == 0 && dma == 1 && dir == 0)
    cmd = ATA_CMD_READ_DMA;
  if (lbaMode == 1 && dma == 1 && dir == 0)
    cmd = ATA_CMD_READ_DMA;
  if (lbaMode == 2 && dma == 1 && dir == 0)
    cmd = ATA_CMD_READ_DMA_EXT;
  if (lbaMode == 0 && dma == 0 && dir == 1)
    cmd = ATA_CMD_WRITE_PIO;
  if (lbaMode == 1 && dma == 0 && dir == 1)
    cmd = ATA_CMD_WRITE_PIO;
  if (lbaMode == 2 && dma == 0 && dir == 1)
    cmd = ATA_CMD_WRITE_PIO_EXT;
  if (lbaMode == 0 && dma == 1 && dir == 1)
    cmd = ATA_CMD_WRITE_DMA;
  if (lbaMode == 1 && dma == 1 && dir == 1)
    cmd = ATA_CMD_WRITE_DMA;
  if (lbaMode == 2 && dma == 1 && dir == 1)
    cmd = ATA_CMD_WRITE_DMA_EXT;

  ideOut8(ch, ATA_REG_COMMAND, cmd);

  if (!dma) {
    if (dir == 0) {
      /* PIO READ */
      for (i = 0; i < sectAmount; i++) {
        u8 err = idePoll(ch, 1);
        if (err) {
          debug("ide: DEVICE FAULT\n");
        }
        asm("rep insw" : : "c"(words), "d"(bus), "D"(buf));
        buf += (words * 2);
      }
    } else {
      for (i = 0; i < sectAmount; i++) {
        idePoll(ch, 0);
        asm("rep outsw" : : "c"(words), "d"(bus), "S"(buf));
        buf += (words * 2);
      }
      ideOut8(ch, ATA_REG_COMMAND,
              (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH,
                       ATA_CMD_CACHE_FLUSH_EXT

              }[lbaMode]);
      idePoll(ch, 0);
    }
  } else {
    debug("ide: UNSUPPORTED");
  }
  return 0;
}

void ideRead(u8 disk, u32 lba, u8 sectAmount, void* buf) {
  if (disk > 3 || !dev[disk].Reserved) {
    debug("ide: TRYING TO READ FROM A NONEXISTENT DRIVE\n");
    return;
  } else if (((lba + sectAmount) > dev[disk].Size) && dev[disk].Type == 0) {
    debug("ide: READING TO INVALID POSITION\n");
    return;
  } else {
    if (dev[disk].Type == 0) {
      ideAccessAta(ATA_READ, disk, lba, sectAmount, (u16*)buf);
    } else {
      debug("ide: READING FROM UNSUPPORTED DISK TYPE (for now)\n");
    }
  }
}
void ideEnum() {
  int i, j, cnt = 0;

  for (i = 0; i < 2; i++) {
    for (j = 0; j < 2; j++) {
      u8 err = 0, type = 0, status = 0;
      dev[cnt].Reserved = 0;

      ideOut8(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
      ideSleep();
      ideOut8(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
      ideSleep();
      if (ideIn8(i, ATA_REG_STATUS) == 0)
        continue; /* no device */
      while (1) {
        status = ideIn8(i, ATA_REG_STATUS);
        if ((status & ATA_SR_ERR)) {
          err = 1;
          break;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
          break;
      }
      if (err != 0) {
        u8 l = ideIn8(i, ATA_REG_LBA1);
        u8 h = ideIn8(i, ATA_REG_LBA2);

        if (l == 0x14 && h == 0xEB) {
          type = 1;
        } else if (l == 0x69 && h == 0x96) {
          type = 1;
        } else {
          continue;
        }
        ideOut8(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
        ideSleep();
      }
      ideInBuf(i, ATA_REG_DATA, (u32*)buf, 128);

      dev[cnt].Reserved = 1;
      dev[cnt].Type = type;
      dev[cnt].Channel = i;
      dev[cnt].Drive = j;
      dev[cnt].Sign = *((u16*)(buf + ATA_IDENT_DEVICETYPE));
      dev[cnt].Capabilities = *((u16*)(buf + ATA_IDENT_CAPABILITIES));
      dev[cnt].CmdSets = *((u32*)(buf + ATA_IDENT_COMMANDSETS));

      if (dev[cnt].CmdSets & (1 << 26)) {
        dev[cnt].Size = *((u32*)(buf + ATA_IDENT_MAX_LBA_EXT));
      } else {
        dev[cnt].Size = *((u32*)(buf + ATA_IDENT_MAX_LBA));
      }

      /* read in model */
      int k;
      for (k = 0; k < 40; k += 2) {
        dev[cnt].Model[k] = buf[ATA_IDENT_MODEL + k + 1];
        dev[cnt].Model[k + 1] = buf[ATA_IDENT_MODEL + k];
        if (!dev[cnt].Model[k])
          break;
      }
      dev[cnt].Model[k] = 0;

      cnt++;
    }
  }
}
void ideInit(u32 bar0, u32 bar1, u32 bar2, u32 bar3, u32 bar4) {
  memset(buf, 0, sizeof(buf));

  channels[0].Base = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
  channels[0].Ctrl = (bar1 & 0xFFFFFFFC) + 0x3F4 * (!bar1);
  channels[1].Base = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
  channels[1].Ctrl = (bar3 & 0xFFFFFFFC) + 0x374 * (!bar3);
  channels[0].BMIde = (bar4 & 0xFFFFFFFC) + 0;
  channels[1].BMIde = (bar4 & 0xFFFFFFFC) + 8;

  ideOut8(0, ATA_REG_CONTROL, 2);
  ideOut8(1, ATA_REG_CONTROL, 2);

  ideEnum();

  int i = 0;
  for (i = 0; i < 4; i++) {
    if (dev[i].Reserved == 1) {
      debug("ide: FOUND IDE: TYPE:%s SIZE:%d MODEL:%s\n",
            (const char*[]){"ATA", "ATAPI"}[dev[i].Type], dev[i].Size,
            dev[i].Model);
    }
  }
}