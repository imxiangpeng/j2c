// mxp, 20231229, json <> struct utils
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
    printf("open failed:%s, error:%s\n", path, strerror(errno));
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

static size_t _write_file(const char *path, char *data, size_t size) {
    int fd = -1;

    char *ptr = data;

    if (!data || !path || size <= 0) {
        return -1;
    }

    fd = open(path, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return -1;
    }

    size_t left = size;
    while (left > 0) {
        ssize_t n = TEMP_FAILURE_RETRY(write(fd, ptr, left));
        if (n == -1) {
            return size - left;
        }
        ptr += n;
        left -= n;
    }
    
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
#if 0
struct j2sobject *j2sobject_create_removed(enum j2stype type, size_t size) {
    struct j2sobject *object = (struct j2sobject *)calloc(1, size);

    switch (type) {
        case J2S_OBJECT:
            J2SOBJECT(object)->type = type;
            J2SOBJECT(object)->name = "object";
            break;
        case J2S_ARRAY:
            J2SOBJECT(object)->type = type;
            J2SOBJECT(object)->name = "array";
            break;
        default:
            break;
    }

    return object;
}
#endif
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

    proto->ctor(self);
    
    // setup object proto only when construct not do
    if (!self->proto)
        self->proto = proto;

    return self;
}
void j2sobject_free(struct j2sobject *self) {
    if (!self || !self->proto || !self->proto->dtor) {
        return;
    }
    
    self->proto->dtor(self);

    free(self);
}

int j2sobject_reset(struct j2sobject *self) {
    if (!self || !self->proto) return -1;

    memset((void*)((char*)self + sizeof(struct j2sobject)), 0, self->proto->size - sizeof(struct j2sobject));

    return 0;
}
#if 0
void j2sobject_free(struct j2sobject *self) {
    if (!self) {
        return;
    }

    if (self->priv_data) {
        cJSON *obj = (cJSON *)self->priv_data;
        obj->type &= ~cJSON_IsReference;
        cJSON_Delete(obj);
        self->priv_data = NULL;
    }
    if (self->proto) {
        const struct j2sobject_fields_prototype *pt = self->proto;

        for (; pt->name != NULL; pt++) {
            switch (pt->type) {
                case J2S_STRING: {
                    // we force using char[], should do not need free here
                    if (pt->offset_len == 0) {  // char*
                        char *ptr = *(char **)((char *)self + pt->offset);
                        free(ptr);
                    }
                    break;
                }
                case J2S_OBJECT: {
                    if (pt->offset_len == 0) {
                        struct j2sobject *ptr = *(struct j2sobject **)((char *)self + pt->offset);
                        j2sobject_free(ptr);
                    }
                    break;
                }
                case J2S_ARRAY:
                default:
                    break;
            }
        }
    }

    free(self);
    return;
}
#endif

// not support inner loop
int j2sobject_deserialize_cjson(struct j2sobject *self, cJSON *jobj) {
    if (!jobj || !self) return -1;

    cJSON *ele = NULL;

    if (0 == cJSON_IsObject(jobj)) {
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
                    //child = pt->proto->ctor();
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
                printf("deserialize: current we do not support array ...\n");
            } break;

            default:
                break;
        }
    }

    return 0;
error:
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

    ret = j2sobject_deserialize_cjson(self, obj);
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

    ret = j2sobject_deserialize_cjson(self, root);
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
                    pt->proto->ctor(object);
                }
                j2sobject_serialize_cjson(object, child);

                break;
            }
            case J2S_ARRAY: {
                break;
            }
            default:
                printf("not support object or array data !\n");
                return -1;
        }
    }

    return 0;
}

// please free the memory
char *j2sobject_serialize(struct j2sobject *self) {
    char *data = NULL;

    if (!self || !self->field_protos) {
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    j2sobject_serialize_cjson(self, root);
    // should be freed manual ...
    data = cJSON_Print(root);
    cJSON_Delete(root);

    return data;
}

int j2sobject_serialize_file(struct j2sobject *self, const char *path) {
    char *data = NULL;
    if (!self || !path) {
        return -1;
    }

    data = j2sobject_serialize(self);

    if (!data) {
        printf("can not got data ...\n");
        return -1;
    }

    _write_file(path, data, strlen(data));

    free(data);

    return 0;
}
