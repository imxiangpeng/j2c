#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

#include "j2cobject.h"

#include "cjson/cJSON.h"

// create not support none name element
// array string/int, such as [ 1, 2 ] or  ["xxx", "yyy"]


#ifndef TEMP_FAILURE_RETRY
    #define TEMP_FAILURE_RETRY(exp)                                                \
  ({                                                                           \
    typeof(exp) _rc;                                                           \
    do {                                                                       \
      _rc = (exp);                                                             \
    } while (_rc == -1 && errno == EINTR);                                     \
    _rc;                                                                       \
  })
#endif
// you must free the pointer
static size_t _read_file(const char *path, char **buf) {
    int fd = -1;
    size_t length = 0;
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

    return size;
}

struct j2cobject* j2cobject_create(enum j2ctype type, size_t size) {
    struct j2cobject *object = (struct j2cobject *)calloc(1, size);

    switch (type) {
    case J2C_OBJECT:
        J2COBJECT(object)->type = type;
        J2COBJECT(object)->name = "object";
        break;
    case J2C_ARRAY:
        J2COBJECT(object)->type = type;
        J2COBJECT(object)->name = "array";
        break;

    default:
        break;
    }

    return object;
}

void j2cobject_free(struct j2cobject *self) {

    if (!self) {
        return;
    }

    if (self->raw_data) {
        free(self->raw_data);
        self->raw_data = NULL;
    }
    if (self->priv_data) {
        cJSON *obj = (cJSON *)self->priv_data;
        obj->type &= ~cJSON_IsReference;
        cJSON_Delete(obj);
        self->priv_data = NULL;
    }
    if (self->prototype) {
        const struct j2cobject_prototype *pt = self->prototype;

        for (; pt->name != NULL; pt++) {
            switch (pt->type) {
            case J2C_STRING:
                {
                    char *ptr = *(char **)((char *)self + pt->offset);
                    free(ptr);
                    break;
                }
            case J2C_ARRAY:
            case J2C_OBJECT:
                {
                    struct j2cobject *ptr = *(struct j2cobject **)((char *)self + pt->offset);
                    j2cobject_free(ptr);
                    break;
                }
            default:
                break;
            }
        }
    }

    free(self);
    return;
}
static const struct j2cobject_prototype*
j2cobject_field_prototype(struct j2cobject *self, const char *field) {
    const struct j2cobject_prototype *pt = NULL;

    if (!field || !self || !self->prototype) {
        return NULL;
    }

    pt = self->prototype;

    for (; pt->name != NULL; pt++) {
        if (strcmp(pt->name, field) == 0) {
            return pt;
        }
    }
    return NULL;
}

// not support inner loop
static int j2cobject_from_jobject(cJSON *jobj, struct j2cobject *cobj) {
    if (!jobj || !cobj) return -1;

    cJSON *ele = NULL;

    if (0 == cJSON_IsObject(jobj)) {
        return -1;
    }
    cJSON_ArrayForEach(ele, jobj) {
        const struct j2cobject_prototype *pt = NULL;
        if (!ele || !ele->string) continue;

        pt = j2cobject_field_prototype(J2COBJECT(cobj), ele->string);
        if (!pt) {
            printf("can not find key:%s\n", ele->string);
            goto error;
        }

        switch (ele->type) {
        case cJSON_Number:
            {
                if (pt->type == J2C_INT) {
                    int *ptr = (int *)((char *)cobj + pt->offset);
                    *ptr = (int)cJSON_GetNumberValue(ele);
                } else if (pt->type == J2C_DOUBLE) {
                    double *ptr = (double *)((char *)cobj + pt->offset);
                    *ptr = cJSON_GetNumberValue(ele);
                }

                break;
            }
        case cJSON_String:
            {
                // do not free cJSON_Print memory, it willed be freed when object is deallocate
                char **ptr = (char **)((char *)cobj + pt->offset);
                *ptr = strdup(cJSON_GetStringValue(ele));
                break;
            }
        case cJSON_Object:
            {
                // do not free cJSON_Print memory, it willed be freed when object is deallocate
                char **ptr = (char **)((char *)cobj + pt->offset);
                char *str = cJSON_Print(ele);
                *ptr = str;
            }
            break;
        case cJSON_Array:
            {
                // do not free cJSON_Print memory, it willed be freed when object is deallocate
                struct j2cobject **ptr = (struct j2cobject **)((char *)cobj + pt->offset);

                struct j2cobject *arr = (struct j2cobject *)j2cobject_create(J2C_ARRAY, sizeof(struct j2cobject));
                arr->size = cJSON_GetArraySize(ele);
                arr->priv_data = (void *)cJSON_Duplicate(ele, 1);
                *ptr = J2COBJECT(arr);
            }
            break;

        default:
            break;
        }
    }

    return 0;
error:
    return -1;
}

int j2cobject_from_string(const char *jstr, struct j2cobject *cobj) {
    int ret = -1;
    if (!jstr || !cobj) {
        return -1;
    }

    cJSON *obj = cJSON_Parse(jstr);
    if (!obj) {
        return -1;
    }

    ret = j2cobject_from_jobject(obj, cobj);
    cJSON_Delete(obj);

    return ret;
}

int j2cobject_from_file(const char *path, struct j2cobject *cobj) {
    int ret = -1;
    size_t len = 0;
    char *data = NULL;
    if (!path || !cobj) {
        return -1;
    }

    len = _read_file(path, &data);
    if (!data) {
        printf("can not read file:%s\n", path);
        return -1;
    }
    cJSON *obj = cJSON_ParseWithLength(data, len);
    if (!obj) {
        printf("can not read file:%s, data:%s\n", path, data);

        printf("error:%s\n", cJSON_GetErrorPtr());
        free(data);
        return -1;
    }

    ret = j2cobject_from_jobject(obj, cobj);
    cJSON_Delete(obj);

    free(data);
    return ret;
}


int j2cobject_write_file(struct j2cobject *self, const char *path) {
    char *data = NULL;
    if (!self || !path) {
        return -1;
    }

    data = j2cobject_serializer(self);

    if (!data) {
        printf("can not got data ...\n");
        return -1;
    }

    _write_file(path, data, strlen(data));

    free(data);

    return 0;
}
struct j2cobject* j2cobject_to_list(struct j2cobject *object, j2cobject_allocate_handler allocater) {

    struct j2cobject *head = NULL;
    int size = object->size;
    cJSON *json = NULL;
    cJSON *ele = NULL;


    if (!object || !allocater) {
        return NULL;
    }

    if (object->type != J2C_ARRAY) {
        return NULL;
    }


    if (object->priv_data) {
        json = (cJSON *)object->priv_data;
    } else {
        cJSON_Parse(object->raw_data);
    }

    if (!json) {
        return NULL;
    }

    cJSON_ArrayForEach(ele, json) {
        struct j2cobject *subobj = NULL;
        const struct j2cobject_prototype *pt = NULL;
        if (!ele || !cJSON_IsObject(ele)) continue;

        // array string/int, such as [ 1, 2 ] or  ["xxx", "yyy"]

        subobj = allocater();

        if (!head) {
            head = subobj;
            j2cobject_list_init(head);
        } else {
            j2cobject_list_append(head, subobj);
        }

        int ret = j2cobject_from_jobject(ele, subobj);
        if (ret != 0) {
            printf("json convert failed ...\n");
        }
    }

    return head;
}

// please free the memory
char* j2cobject_serializer(struct j2cobject *self) {
    char *data = NULL;
    const struct j2cobject_prototype *pt = NULL;

    if (!self || !self->prototype) {
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    pt = self->prototype;

    for (; pt->name != NULL; pt++) {
        char **ptr = (char **)((char *)self + pt->offset);
        switch (pt->type) {
        case J2C_INT:
            {
                int num = *(int *)((char *)self + pt->offset);
                cJSON_AddNumberToObject(root, pt->name, num);
                break;
            }
        case J2C_DOUBLE:
            {
                double num = *(double *)((char *)self + pt->offset);
                cJSON_AddNumberToObject(root, pt->name, num);
                break;
            }
        case J2C_STRING:
            {
                char *str = *ptr;
                cJSON *ele = cJSON_AddStringToObject(root, pt->name, str);
                if (!ele) {
                    // failed we should release
                    cJSON_Delete(root);
                    return NULL;
                }
                break;
            }
        case J2C_ARRAY:
        case J2C_OBJECT:
            {
                struct j2cobject *object = (struct j2cobject *)*ptr;
                if (!object) {
                    // using empty object/array, do not return error
                    if (pt->type == J2C_OBJECT) {
                        cJSON_AddObjectToObject(root, pt->name);
                    } else {
                        cJSON_AddArrayToObject(root, pt->name);
                    }
                    continue;
                }

                if (object->priv_data) {
                    cJSON *o = (cJSON *)object->priv_data;
                    cJSON_AddItemToObject(root, pt->name, cJSON_Duplicate(o, 1));
                } else if (object->raw_data) {
                    // fallback using raw data method
                    // so user can create array object and fill raw_data
                    // no need call cJSON_Delete(o), because it has been added into root
                    cJSON *o = cJSON_Parse(object->raw_data);
                    if (o) {
                        if (!cJSON_AddItemToObject(root, pt->name, o)) {
                            // delete it when it's not added into root
                            cJSON_Delete(o);
                        }
                    }
                }
                break;
            }

        }
    }

    // should be freed manual ...
    data = cJSON_Print(root);
    cJSON_Delete(root);

    return data;
}
