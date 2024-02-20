// mxp, 20231229, json <> struct utils
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "j2sobject.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cjson/cJSON.h"

#define TMPFILE_TEMPLATE ".tmp_XXXXXX"

// create not support none name element
// array string/int, such as [ 1, 2 ] or  ["xxx", "yyy"]

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)                \
    ({                                         \
        typeof(exp) _rc;                       \
        do {                                   \
            _rc = (exp);                       \
        } while (_rc == -1 && errno == EINTR); \
        _rc;                                   \
    })
#endif
// you must free the pointer
static size_t _read_file(const char *path, char **buf) {
    int fd = -1;
    struct stat sb;
    char *data = NULL, *ptr = NULL;
    if (lstat(path, &sb) != 0 || sb.st_size == 0 || !buf) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    data = (char *)malloc(sb.st_size);
    if (!data) {
        close(fd);
        return -1;
    }
    memset((void *)data, 0, sb.st_size);

    ptr = data;

    size_t remaining = sb.st_size;
    while (remaining > 0) {
        ssize_t n = TEMP_FAILURE_RETRY(read(fd, ptr, remaining));
        if (n <= 0) {
            free(ptr);
            return -1;
        }
        ptr += n;
        remaining -= n;
    }
    close(fd);

    *buf = data;
    return sb.st_size;
}

static ssize_t _write_file_fd(int fd, char *data, size_t size) {
    ssize_t left = size;
    char *ptr = data;

    if (fd < 0 || !data || size == 0) return -1;

    while (left > 0) {
        ssize_t n = TEMP_FAILURE_RETRY(write(fd, ptr, left));
        if (n == -1) {
            close(fd);
            return size - left;
        }
        ptr += n;
        left -= n;
    }

    return size;
}
static ssize_t _write_file(const char *path, char *data, size_t size) {
    int fd = -1;

    if (!data || !path || size <= 0) {
        return -1;
    }

    fd = open(path, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return -1;
    }

    size = _write_file_fd(fd, data, size);
    close(fd);

    return size;
}

static const struct j2sobject_fields_prototype *
j2sobject_field_prototype(struct j2sobject *self, const char *field) {
    const struct j2sobject_fields_prototype *pt = NULL;

    if (!field || !self || !self->field_protos) {
        return NULL;
    }

    pt = self->field_protos;

    for (; pt->name != NULL; pt++) {
        if (strcmp(pt->name, field) == 0) {
            return pt;
        }
    }
    return NULL;
}

struct j2sobject *j2sobject_create(struct j2sobject_prototype *proto) {
    struct j2sobject *self = NULL;
    if (!proto)
        return NULL;
    if (proto->type != J2S_OBJECT) {
        printf("now only support J2S_OBJECT, not support:%d ...\n", proto->type);
        return NULL;
    }

    if (!proto->ctor || proto->size == 0) {
        printf("curr object:%s does not support construct dynamic ...\n", proto->name ? proto->name : "unknown");
        return NULL;
    }

    self = (struct j2sobject *)calloc(1, proto->size);

    // must special array & proto
    self->type = J2S_OBJECT;

    proto->ctor(self);

    // setup object proto only when construct not do
    if (!self->proto)
        self->proto = proto;

    // init list
    self->next = self->prev = self;
    return self;
}

// create object array
struct j2sobject *j2sobject_create_array(struct j2sobject_prototype *proto) {
    struct j2sobject *self = NULL;

    if (!proto)
        return NULL;

    if (proto->type != J2S_OBJECT) {
        printf("now only support J2S_OBJECT, not support:%d ...\n", proto->type);
        return NULL;
    }

    if (!proto->ctor || proto->size == 0) {
        printf("curr object:%s does not support construct dynamic ...\n", proto->name ? proto->name : "unknown");
        return NULL;
    }

    self = (struct j2sobject *)calloc(1, proto->size);

    // must special array & proto
    self->type = J2S_ARRAY;
    // using proto as subobject proto, so we known child type
    self->proto = proto;

    // must init list
    self->next = self->prev = self;
    return self;
}

void j2sobject_free(struct j2sobject *self) {
    const struct j2sobject_fields_prototype *pt = NULL;
    if (!self || !self->proto) {
        return;
    }

    if (self->type == J2S_ARRAY) {
        struct j2sobject *p = NULL, *n = NULL;
        // loop all elements free all element
        for (p = self->next, n = p->next; p != self; p = n, n = p->next) {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            j2sobject_free(p);
        }

        memset((void *)self, 0, sizeof(struct j2sobject));
        free(self);
        return;
    }

    // should release all elements
    pt = self->field_protos;

    if (!pt) {
        return;
    }

    for (; pt->name != NULL; pt++) {
        switch (pt->type) {
            case J2S_STRING: {
                // char* -> char**
                // char [] -> char*
                if (pt->offset_len == 0) {  // char*
                    char **str = (char **)((char *)self + pt->offset);
                    if (*str) {
                        free(*str);
                        *str = NULL;  // force clear the member
                    }
                }

                break;
            }
            case J2S_ARRAY:
            case J2S_OBJECT: {
                struct j2sobject *object = NULL;
                // pt->offset_len 0: -> pointer
                //              > 0: -> struct data
                if (pt->offset_len == 0) {
                    struct j2sobject **ptr = (struct j2sobject **)((char *)self + pt->offset);
                    if (*ptr != NULL) {
                        j2sobject_free(*ptr);
                        *ptr = NULL;  // force clear the member
                    }
                } else {
                    object = (struct j2sobject *)((char *)self + pt->offset);
                    // care that if object contains char* memory(which maybe allocated in deserialize),
                    // you should release it, because we can not call j2sobject_free here
                    if (pt->proto->dtor)
                        pt->proto->dtor(object);
                }
                break;
            }
            default:
                break;
        }
    }

    if (self->proto->dtor)
        self->proto->dtor(self);

    self->next = self->prev = NULL;
    free(self);
}

int j2sobject_reset(struct j2sobject *self) {
    if (!self || !self->proto) return -1;

    memset((void *)((char *)self + sizeof(struct j2sobject)), 0, self->proto->size - sizeof(struct j2sobject));

    return 0;
}

// not support inner loop
int j2sobject_deserialize_cjson(struct j2sobject *self, cJSON *jobj) {
    if (!jobj || !self) return -1;

    cJSON *ele = NULL;

    if (!cJSON_IsObject(jobj)) {
        return -1;
    }
    cJSON_ArrayForEach(ele, jobj) {
        const struct j2sobject_fields_prototype *pt = NULL;
        if (!ele || !ele->string) continue;

        pt = j2sobject_field_prototype(J2SOBJECT(self), ele->string);
        if (!pt) {
            printf("can not find key:%s\n", ele->string);
            goto error;
        }

        switch (ele->type) {
            case cJSON_Number: {
                if (pt->type == J2S_INT) {
                    int *ptr = (int *)((char *)self + pt->offset);
                    *ptr = (int)cJSON_GetNumberValue(ele);
                } else if (pt->type == J2S_DOUBLE) {
                    double *ptr = (double *)((char *)self + pt->offset);
                    *ptr = cJSON_GetNumberValue(ele);
                }

                break;
            }
            case cJSON_String: {
                // do not free cJSON_Print memory, it willed be freed when object is deallocate
                if (pt->offset_len == 0) {  // char *
                    char **ptr = (char **)((char *)self + pt->offset);
                    *ptr = strdup(cJSON_GetStringValue(ele));
                } else {  // char[]
                    char *ptr = (char *)((char *)self + pt->offset);
                    // make sure no data loss
                    if (strlen(cJSON_GetStringValue(ele)) > pt->offset_len - 1) {
                        printf("string value too long ..., allow max:%d\n", pt->offset_len);
                        goto error;
                    }
                    snprintf(ptr, pt->offset_len, "%s", cJSON_GetStringValue(ele));
                }
                break;
            }
            case cJSON_Object: {
                // pt->offset_len 0: -> pointer
                //              > 0: -> struct data
                struct j2sobject *child = NULL;
                if (pt->offset_len == 0) {
                    struct j2sobject **ptr = (struct j2sobject **)((char *)self + pt->offset);
                    child = j2sobject_create(pt->proto);
                    *ptr = child;
                } else {
                    child = (struct j2sobject *)((char *)self + pt->offset);
                    // must call init to setup prototype
                    pt->proto->ctor(child);
                }
                j2sobject_deserialize_cjson(child, ele);
            } break;
            case cJSON_Array: {
                // array 's proto is array subobject's proto
                struct j2sobject *child = NULL;
                if (pt->offset_len == 0) {
                    struct j2sobject **ptr = (struct j2sobject **)((char *)self + pt->offset);
                    child = j2sobject_create_array(pt->proto);
                    *ptr = child;
                } else {
                    child = (struct j2sobject *)((char *)self + pt->offset);
                    // must call init to setup prototype
                    pt->proto->ctor(child);
                }
                j2sobject_deserialize_array_cjson(child, ele);
            } break;

            default:
                break;
        }
    }

    return 0;
error:
    return -1;
}

// j2sobject array should contains child's proto, otherwise we can not create
// child object
// we only support [{},{}]
// not support [1,2,3,...] or ["x","y",...]
int j2sobject_deserialize_array_cjson(struct j2sobject *self, cJSON *jobj) {
    cJSON *ele = NULL;
    struct j2sobject *p = NULL, *n = NULL;
    if (!self || self->type != J2S_ARRAY || !jobj || !cJSON_IsArray(jobj)) {
        return -1;
    }

    // assert self's prev/next have been inited

    cJSON_ArrayForEach(ele, jobj) {
        struct j2sobject *child = j2sobject_create(self->proto);
        // add to array list
        self->prev->next = child;
        child->prev = self->prev;
        child->next = self;
        self->prev = child;
        if (j2sobject_deserialize_cjson(child, ele) != 0) {
            goto failed;
        }
    }
    return 0;

failed:
    // loop all elements free all element
    for (p = self->next, n = p->next; p != self; p = n, n = p->next) {
        j2sobject_free(p);
        p->prev->next = p->next;
        p->next->prev = p->prev;
    }
    return -1;
}

int j2sobject_deserialize(struct j2sobject *self, const char *jstr) {
    int ret = -1;
    if (!jstr || !self) {
        return -1;
    }

    cJSON *obj = cJSON_Parse(jstr);
    if (!obj) {
        return -1;
    }

    if (cJSON_IsArray(obj)) {
        ret = j2sobject_deserialize_array_cjson(self, obj);
    } else {
        ret = j2sobject_deserialize_cjson(self, obj);
    }
    cJSON_Delete(obj);

    return ret;
}

int j2sobject_deserialize_file(struct j2sobject *self, const char *path) {
    int ret = -1;
    size_t len = 0;
    char *data = NULL;
    if (!path || !self) {
        return -1;
    }

    len = _read_file(path, &data);
    if (!data) {
        printf("can not read file:%s\n", path);
        return -1;
    }
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) {
        printf("can not read file:%s, data:%s\n", path, data);

        printf("error:%s\n", cJSON_GetErrorPtr());
        free(data);
        return -1;
    }
    if (cJSON_IsArray(root)) {
        ret = j2sobject_deserialize_array_cjson(self, root);
    } else {
        ret = j2sobject_deserialize_cjson(self, root);
    }
    cJSON_Delete(root);

    free(data);
    return ret;
}
// only support basic data type
int j2sobject_serialize_cjson(struct j2sobject *self, struct cJSON *target) {
    cJSON *root = target;

    if (!self || !self->field_protos || !root) {
        return -1;
    }

    const struct j2sobject_fields_prototype *pt = NULL;
    pt = self->field_protos;

    for (; pt->name != NULL; pt++) {
        switch (pt->type) {
            case J2S_INT: {
                int num = *(int *)((char *)self + pt->offset);
                cJSON_AddNumberToObject(root, pt->name, num);
                break;
            }
            case J2S_DOUBLE: {
                double num = *(double *)((char *)self + pt->offset);
                cJSON_AddNumberToObject(root, pt->name, num);
                break;
            }
            case J2S_STRING: {
                // char* -> char**
                // char [] -> char*
                char *str = NULL;
                if (pt->offset_len == 0) {  // char*
                    str = *(char **)((char *)self + pt->offset);
                } else {  // char[]
                    str = ((char *)self + pt->offset);
                }

                cJSON *ele = cJSON_AddStringToObject(root, pt->name, str);
                if (!ele) {
                    // failed we should release
                    return -1;
                }
                break;
            }
            case J2S_OBJECT: {
                struct j2sobject *object = NULL;

                cJSON *child = cJSON_AddObjectToObject(root, pt->name);
                if (pt->offset_len == 0) {
                    object = *(struct j2sobject **)((char *)self + pt->offset);
                } else {
                    object = (struct j2sobject *)((char *)self + pt->offset);
                }
                // must manual init object's fields
                // when it's sub struct, the header maybe invalid
                if (!object->name) {
                    pt->proto->ctor(object);
                }
                j2sobject_serialize_cjson(object, child);

                break;
            }
            case J2S_ARRAY: {
                struct j2sobject *object = NULL;
                cJSON *array = cJSON_CreateArray();
                cJSON_AddItemToObject(root, pt->name, array);
                if (pt->offset_len == 0) {
                    object = *(struct j2sobject **)((char *)self + pt->offset);
                } else {
                    object = (struct j2sobject *)((char *)self + pt->offset);
                }
                // must manual init object's fields
                // when it's sub struct, the header maybe invalid
                if (!object->name) {
                    pt->proto->ctor(object);
                }

                j2sobject_serialize_array_cjson(object, array);

                break;
            }
            default:
                printf("not support object or array data !\n");
                return -1;
        }
    }

    return 0;
}

int j2sobject_serialize_array_cjson(struct j2sobject *self, struct cJSON *target) {
    struct j2sobject *e = NULL;
    if (!self || self->type != J2S_ARRAY || !target || !cJSON_IsArray(target)) return -1;

    // loop all elements free all element
    for (e = self->next; e != self; e = e->next) {
        cJSON *object = cJSON_CreateObject();
        j2sobject_serialize_cjson(e, object);
        cJSON_AddItemToArray(target, object);
    }

    return 0;
}

// please free the memory
char *j2sobject_serialize(struct j2sobject *self) {
    char *data = NULL;
    cJSON *root = NULL;

    if (!self || !self->proto) {
        return NULL;
    }

    if (self->type == J2S_ARRAY) {
        root = cJSON_CreateArray();
        j2sobject_serialize_array_cjson(self, root);
    } else if (self->type == J2S_OBJECT) {
        if (!self->field_protos) return NULL;
        root = cJSON_CreateObject();
        j2sobject_serialize_cjson(self, root);
    } else {
        return NULL;
    }
    // should be freed manual ...
    if (root) {
        data = cJSON_Print(root);
        cJSON_Delete(root);
    }

    return data;
}

int j2sobject_serialize_file(struct j2sobject *self, const char *path) {
    int fd = -1;
    char *data = NULL;
    char *tmp = NULL;
    int tmp_len = 0;
    if (!self || !path) {
        return -1;
    }

    tmp_len = strlen(path) + strlen(TMPFILE_TEMPLATE) + 1;  // + '\0'

    tmp = (char *)calloc(1, tmp_len);  // hardcode 8(.XXXXXX + \0)
    if (!tmp) return -1;

    snprintf(tmp, tmp_len, "%s%s", path, TMPFILE_TEMPLATE);
    fd = mkostemp(tmp, O_RDWR | O_TRUNC | O_CREAT);
    if (fd < 0) {
        free(tmp);
        return -1;
    }

    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP);

    data = j2sobject_serialize(self);
    if (!data) {
        unlink(tmp);
        free(tmp);
        close(fd);
        return -1;
    }

    _write_file_fd(fd, data, strlen(data));

    close(fd);

    unlink(path);
    rename(tmp, path);

    free(tmp);
    free(data);

    return 0;
}
