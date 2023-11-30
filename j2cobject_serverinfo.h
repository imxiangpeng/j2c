/*file is auto created , do not modify it!!!*/
/*
* Copyright (C) 2023 Inspur Group Co., Ltd. Unpublished
*
* Inspur Group Co., Ltd.
* Proprietary & Confidential
*
* This source code and the algorithms implemented therein constitute
* confidential information and may comprise trade secrets of Inspur
* or its associates, and any use thereof is subject to the terms and
* conditions of the Non-Disclosure Agreement pursuant to which this
* source code was originally received.
*/

#ifndef _J2COBJECT_SERVERINFO_H_
#define _J2COBJECT_SERVERINFO_H_

#include "stddef.h"
#include "stdlib.h"
#include "j2cobject.h"


struct j2cobject_serverinfo {
  J2COBJECT_DECLARE_OBJECT;
  int int_val;
  char* bssaddr;
  char* abiaddr;
  char* devtoken;
  char* devid;
  struct j2cobject* backup;
  struct j2cobject* macs;
  double double_val;
};

static struct j2cobject_prototype j2cobject_prototype_serverinfo[] = {
  {
    .name= "int_val",
    .type= J2C_INT,
    .offset= offsetof(struct j2cobject_serverinfo, int_val),
    .offset_len= 0
  },
  {
    .name= "bssaddr",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_serverinfo, bssaddr),
    .offset_len= 0
  },
  {
    .name= "abiaddr",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_serverinfo, abiaddr),
    .offset_len= 0
  },
  {
    .name= "devtoken",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_serverinfo, devtoken),
    .offset_len= 0
  },
  {
    .name= "devid",
    .type= J2C_STRING,
    .offset= offsetof(struct j2cobject_serverinfo, devid),
    .offset_len= 0
  },
  {
    .name= "backup",
    .type= J2C_ARRAY,
    .offset= offsetof(struct j2cobject_serverinfo, backup),
    .offset_len= 0
  },
  {
    .name= "macs",
    .type= J2C_ARRAY,
    .offset= offsetof(struct j2cobject_serverinfo, macs),
    .offset_len= 0
  },
  {
    .name= "double_val",
    .type= J2C_DOUBLE,
    .offset= offsetof(struct j2cobject_serverinfo, double_val),
    .offset_len= 0
  },
  {0}
};

static inline struct j2cobject_serverinfo* j2cobject_serverinfo_allocate() {
  struct j2cobject *obj = j2cobject_create(J2C_OBJECT, sizeof(struct j2cobject_serverinfo));
  if (!obj) return NULL;
  obj->name = "serverinfo";
  obj->prototype = j2cobject_prototype_serverinfo;
  return (struct j2cobject_serverinfo*)obj;
}
static inline void j2cobject_serverinfo_deallocate(struct j2cobject_serverinfo* obj) {
  if (!obj) return;
   j2cobject_free(J2COBJECT(obj));
}
#endif //_J2COBJECT_SERVERINFO_H_
