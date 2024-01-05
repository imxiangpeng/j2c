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

#ifndef _J2SOBJECT_H_
#define _J2SOBJECT_H_

#include <stddef.h>

#define J2SOBJECT_DECLARE_OBJECT struct j2sobject __object__
#define J2SOBJECT(self) ((struct j2sobject *)self)
#define J2SOBJECT_PROTOTYPE(self) J2SOBJECT(self)->prototype

struct cJSON;
struct j2sobject;

enum j2stype {
    J2S_UNKNOWN = 0,
    J2S_INT,
    J2S_DOUBLE,
    J2S_STRING,
    J2S_OBJECT,
    J2S_ARRAY,
};

struct j2sobject_prototype {
    const char *name;
    enum j2stype type; // object type
    int size;
    // only used when it's object not normal basic type such int/string/...
    // so we can create new dynamic object
    int (*ctor)(struct j2sobject *);
    // when offset_len > 0 it means that it's not pointer, data have been allocated
    // you should directly setup proto and other fields
    // care that do not modify any other none j2cobject fields which maybe causing loss data
    //int (*init)(struct j2sobject *);
    int (*dtor)(struct j2sobject *);
};

struct j2sobject_fields_prototype {
    const char *name;
    int type; // object type
    size_t offset;
    size_t offset_len;

    // should not null when cur field is object
    struct j2sobject_prototype *proto;
};

struct j2sobject {
    int type;
    const char *name;

    struct j2sobject_prototype *proto;
    const struct j2sobject_fields_prototype *field_protos;  // point to object prototype table(arrary)

    // store json object/array using inner cJSON object
    //void *priv_data;  // preferred when deserializer

    //size_t size;  // array element numbers
};

//typedef struct j2sobject *(*j2sobject_allocate_handler)();

//struct j2sobject *j2sobject_create(enum j2stype type, size_t size);
struct j2sobject *j2sobject_create(struct j2sobject_prototype *proto);
void j2sobject_free(struct j2sobject *self);

// clear all payload data memory
int j2sobject_reset(struct j2sobject *self);

int j2sobject_deserialize(struct j2sobject *self, const char *jstr);
int j2sobject_deserialize_cjson(struct j2sobject *self, struct cJSON *jobj);
int j2sobject_deserialize_file(struct j2sobject *self, const char *path);
// must be freed manually
char *j2sobject_serialize(struct j2sobject *self);
int j2sobject_serialize_cjson(struct j2sobject *self, struct cJSON *target);
int j2sobject_serialize_file(struct j2sobject *self, const char *path);

#endif  //_J2SOBJECT_H_
