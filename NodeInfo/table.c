#include <limits.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "table.h"
#include "node_pair.h"

#define T Table_T

struct T {
    int size;
    int (*cmp) (const void *x, const void *y);
    unsigned (*hash) (const void *key);
    int length;
    unsigned timestamp;
    struct binding {
        struct binding *link;
        const void *key;
        void *value;
    } **buckets;
};

static int cmpatom(const void *x, const void *y) {
    return x != y;
}

static unsigned hashatom(const void *key) {
    return (unsigned long)key >> 2; // 右移两位是因为可能每个原子都起始于字边界，因此最右侧两位可能是0。
}

T Table_new(int hint, int cmp(const void *x, const void *y), unsigned hash(const void *key)) {
    T table;
    int i;

    static int primes[] = {509, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX};
    assert(hint >= 0);

    for (i = 1; primes[i] < hint; i++) {
        ;
    }
    table = (T)malloc(sizeof(*table) * primes[i-1] * sizeof(table->buckets[0]));
    table->size = primes[i-1];
    table->cmp = cmp ? cmp : cmpatom;
    table->hash = hash ? hash : hashatom;
    table->buckets = (struct binding **)(table + 1);
    for (i = 0;i < table->size;i++) {
        table->buckets[i] = NULL;
    }
    table->length = 0;
    table->timestamp = 0;
    return table;
}

void *Table_get(T table, const void *key) {
    int i;
    struct binding *p;
    assert(table);
    assert(key);

    i = (*table->hash)(key) % table->size;
    for (p = table->buckets[i]; p; p = p->link) {
        if ((*table->cmp)(key, p->key) == 0) {
            break;
        }
    }
    return p ? p->value : NULL;
}

void *Table_put(T table, const void *key, void *value) {
    int i;
    struct binding *p;
    void *prev;

    assert(table);
    assert(key);

    i = (*table->hash)(key) % table->size;
    for (p = table->buckets[i]; p; p = p->link) {
        if ((*table->cmp)(key, p->key) == 0)
            break;
    }

    if (p == NULL) {
        // NEW(p);
        p = malloc(sizeof(struct binding*));
        p->key = key;
        p->link = table->buckets[i];
        table->buckets[i] = p;
        table->length++;
        prev = NULL;
    } else {
        prev = p->value;
    }

    p->value = value;
    table->timestamp++;
    return prev;
}

int Table_length(T table) {
    assert(table);
    return table->length;
}

void *Table_remove(T table, const void *key) {
    int i;
    struct binding **pp;

    assert(table);
    assert(key);
    table->timestamp++;

    i = (*table->hash)(key) % table->size;
    for (pp = &table->buckets[i]; *pp; pp = &(*pp)->link) {
        if ((*table->cmp)(key, (*pp)->key) == 0) {
            struct binding *p = *pp;
            void *value = p->value;
            *pp = p->link;
            if (p != NULL) free(p);
            table->length--;
            return value;
        }
    }
    return NULL;
}

void **Table_toArray(T table, void *end) {
    int i, j = 0;
    void **array;
    struct binding *p;
    assert(table);
    array = malloc((2 * table->length + 1) * sizeof(*array)); 
    for (i = 0;i < table->size;i++) {
        for (p = table->buckets[i]; p; p = p->link) {
            array[j++] = (void *)p->key;
            array[j++] = p->value;
        }
    }
    array[j] = end;
    return array;
}

void Table_free(T *table) {
    assert(table && *table);
    if ((*table)->length > 0) {
        int i;
        struct binding *p, *q;
        for (i = 0;i < (*table)->size;i++) {
            for (p = (*table)->buckets[i]; p; p = q) {
                q = p->link;
                if (p != NULL) free(p);
            }
        }
    }
}

// int main() {
//     struct Node fromNode;
//     struct Node toNode;
//     fromNode.id = 1;
//     toNode.id = 2;
//     P pair = create_node_pair(&fromNode, &toNode, "127.0.0.1");

//     T table = Table_new(3, 0, 0);
//     W w = create_width_delay(50, 500.2);
//     Table_put(table, pair, w);

//     void *get_value = Table_get(table, pair);

//     printf("the value is: %d, width: %lf\n", (*(W)get_value).delay, (*(W)get_value).width);

//     Table_remove(table, pair);

//     get_value = Table_get(table, pair);

//     if (get_value == NULL) {
//         printf("is NULL\n");
//     }
// }