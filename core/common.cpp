#include "common.h"

NAMESPACE_SPL_BEGIN;

void *spl_alloc_low(size_t siz) {
	void *memory = new char[siz];
    if (memory == nullptr)
        throw "Out of memory";
    return memory;
}

void spl_free(void *addr) {
	delete [] addr;
}

NAMESPACE_SPL_END;