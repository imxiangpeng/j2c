// mxp, a very simple json table manipulated using j2sobject

#ifndef _J2STABLE_H_
#define _J2STABLE_H_

#include "j2sobject.h"

#define J2STBL_DECLARE_OBJECT struct j2stbl_object __object__
#define J2STBL_OBJECT_SELF(self) ((struct j2stbl_object *)self)

#define J2STBL_OBJECT_PRIV_FIELDS \
    {.name = "__id__", .type = J2S_INT, .offset = offsetof(struct j2stbl_object, __id__), .offset_len = 0}

enum {
  J2STBL_OPBIT_INSERT = 0x1,
  J2STBL_OPBIT_UPDATE = 0x2,
  J2STBL_OPBIT_DELETE = 0x4
};
struct j2stbl_object {
  J2SOBJECT_DECLARE_OBJECT;
   // auto increase id, readonly, do not modify it!
  int __id__;
};
struct j2stable {
  char *path;

  int state;

  struct j2stbl_object object;
};

struct j2stable *j2stable_init(const char *table, struct j2sobject_prototype *proto);
void j2stable_deinit(struct j2stable *tbl);

int j2stable_empty(struct j2stable *tbl);

// object.id will auto increase ++
int j2stable_insert(struct j2stable *tbl, struct j2stbl_object *self);
#endif // _J2STABLE_H_