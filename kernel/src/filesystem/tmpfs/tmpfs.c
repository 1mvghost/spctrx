#include <printf.h>
#include <tmpfs.h>

static FsHandler tmpfsHandler = {0};

FsNode* tmpfsAllocNode(FsMnt* mnt) {
  FsNode* node = vfsAllocNode(mnt);

  node->ops = &tmpfsHandler;

  return node;
}

void tmpfsInit(FsMnt* mnt) {
  mnt->root = tmpfsAllocNode(mnt);

  printf("tmpfs: new mnt %s\n", mnt->path);
}