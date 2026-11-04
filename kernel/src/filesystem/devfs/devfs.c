#include <devfs.h>
#include <printf.h>
#include <serial.h>
#include <string.h>
#include <tmpfs.h>

/**
 * this is really just a wrapper for tmpfs for now.
 */

static FsHandler devfsHandler = {.write = devfsWrite,
                                 .create = tmpfsCreate,
                                 .lookup = tmpfsLookup,
                                 .mkdir = tmpfsMkdir,
                                 .readdir = tmpfsReaddir};

size_t devfsWrite(FsNode* n, u64 pos, u8* buf, u64 size) {
  UNUSED(pos);

  /**
   * read in the device type
   */
  u8 typeBuffer[32 + 1];

  if (!tmpfsRead(n, 0, typeBuffer, sizeof(typeBuffer) - 1)) {
    return 0;
  }

  char* type = (char*)typeBuffer;

  /**
   * TODO: implement a better way to do this
   */
  if (!strcmp(type, "tty")) {
    for (u64 i = 0; i < size; i++) {
      printf("%c", buf[i]);
    }

    return size;
  }
  return 0;
}

void devfsCreateDev(FsMnt* mnt, char* name) {
  tmpfsCreate(mnt->root, name);

  FsNode* node = tmpfsLookup(mnt->root, name);
  if (node == 0) {
    printf("devfs: tmpfs bug, lookup for created node failed\n");
    return;
  }

  node->ops = &devfsHandler;

  tmpfsWrite(node, 0, (u8*)name, sizeof(name));
}

void devfsMount(FsMnt* mnt) {
  mnt->root = tmpfsAllocNode(mnt, "", TYPE_DIR);
  mnt->root->ops = &devfsHandler;

  devfsCreateDev(mnt, "tty");

  printf("devfs: new mnt %s\n", mnt->path);
}