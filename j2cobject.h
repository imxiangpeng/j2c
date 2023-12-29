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

#ifndef _J2COBJECT_H_
#define _J2COBJECT_H_

#define J2COBJECT_NAME_LEN 32

#define J2COBJECT_DECLARE_OBJECT struct j2cobject __object__
#define J2COBJECT(self) ((struct j2cobject*)self)
#define J2COBJECT_PROTOTYPE(self) J2COBJECT(self)->prototype

struct cJSON;

enum j2ctype {
    J2C_UNKNOWN = 0,
    J2C_INT,
    J2C_DOUBLE,
    J2C_STRING,
    J2C_OBJECT,
    J2C_ARRAY,
};

struct j2cobject_prototype {
    const char *name;
    int type;
    size_t offset;
    size_t offset_len;
    // only used when it's object not normal basic type such int/string/...
    // so we can create new dynamic object
    struct j2cobject* (*ctor)();
    // when offset_len > 0 it means that it's not pointer, data have been allocated
    // you should directly setup proto and other fields
    int (*init)(struct j2cobject *);
    int (*dtor)(struct j2cobject *);
};

struct j2cobject {
    int type;
    const char *name;
    const struct j2cobject_prototype *prototype; // point to object prototype table(arrary)

    // store json object/array using raw string
    char *raw_data;

    // store json object/array using inner cJSON object
    void *priv_data; // preferred when deserializer

    size_t size; // array element numbers

    struct j2cobject *next;
};

typedef struct j2cobject *(*j2cobject_allocate_handler)();

struct j2cobject* j2cobject_create(enum j2ctype type, size_t size);
void j2cobject_free(struct j2cobject *self);

int j2cobject_from_string(const char *jstr, struct j2cobject *cobj);
int j2cobject_from_file(const char *path, struct j2cobject *cobj);
int j2cobject_write_file(struct j2cobject *self, const char *path);
// only support array object
struct j2cobject* j2cobject_to_list(struct j2cobject *object, j2cobject_allocate_handler allocater);
// must be freed manually
char* j2cobject_serializer(struct j2cobject *self);
int j2cobject_serializer_to_cjson(struct j2cobject *self, struct cJSON *target);


int j2cobject_write_file(struct j2cobject *self, const char *path);

static inline void j2cobject_list_init(struct j2cobject *head) {
    head->next = NULL;
}

static inline int j2cobject_list_append(struct j2cobject *head, struct j2cobject *object) {
    struct j2cobject *it = head;
    for (; it->next != NULL; it = it->next) {}

    it->next = object;
    return 0;
}

static inline void j2cobject_list_destroy(struct j2cobject *head) {
    struct j2cobject *loop = head;
    struct j2cobject *node = NULL;

    if (!head) {
        return;
    }

    for (loop = head; loop != NULL;) {
        node = loop;
        loop = loop->next;
        j2cobject_free(node);
    }
}

#define	j2cobject_list_for_each(p, head)						\
	for (p = head; p != NULL; p = p->next)

#endif //_J2COBJECT_H_

