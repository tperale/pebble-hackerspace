#pragma once

#include <pebble.h>

/* @desc : Data transmited by bluetooth can take some time to get to the pebble,
 *      this datastructure make sure to always know the number of element in it.
 */
typedef struct SecureArray {
    void** array;
    uint32_t current;
    uint32_t length;
    void (*add)(struct SecureArray*, void*);
    void* (*get)(struct SecureArray*, int (*)(void*));
    void (*free)(struct SecureArray*, void (*)(void*));
} SecureArray;

/* @desc : Create a new sensor array.
 *
 * @param {uint32_t} : length of the array.
 */
SecureArray* SecureArray_new (uint32_t);
