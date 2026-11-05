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

int devfsWrite(FsNode* n, u64 pos, void* buf, size_t size) {
  UNUSED(pos);

  char* buffer = (char*)buf;

  /**
   * read in the device type
   */
  char type[32 + 1];

  if (tmpfsRead(n, 0, type, sizeof(type) - 1) < 0) {
    return -1;
  }

  /**
   * TODO: implement a better way to do this
   */
  if (!strcmp(type, "tty")) {
    for (u64 i = 0; i < size; i++) {
      printf("%c", buffer[i]);
    }
  }

  return 0;
}

void devfsCreateDev(FsMnt* mnt, char* name) {
  if (tmpfsCreate(mnt->root, name) < 0) {
    printf("devfs: tmpfs bug, node creation failed!\n");
    return;
  }

  FsNode* node = tmpfsLookup(mnt->root, name);
  node->ops = &devfsHandler;

  if (tmpfsWrite(node, 0, name, sizeof(name)) < 0) {
    printf("devfs: tmpfs bug, write to created node failed!\n");
  }
}

void devfsMount(FsMnt* mnt) {
  mnt->root = tmpfsAllocNode(mnt, "", TYPE_DIR);
  mnt->root->ops = &devfsHandler;

  devfsCreateDev(mnt, "tty");

  printf("devfs: new mnt %s\n", mnt->path);
}