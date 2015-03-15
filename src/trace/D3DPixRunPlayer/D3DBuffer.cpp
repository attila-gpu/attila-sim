/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#include "stdafx.h"

#include "D3DBuffer.h"

#include <cstdlib>

D3DBuffer::D3DBuffer(): size(0), data(0) {
}

D3DBuffer::~D3DBuffer() {
    if (data != 0) free(data);
}

void D3DBuffer::setSize(size_t size) {
    // For now we will recreate the whole buffer each time
    if(data != 0) { free(data); }
    data = malloc(size);
    this->size = size;
}

size_t D3DBuffer::getSize() {
    return size;
}

void D3DBuffer::lock(void **p) {
    *p = data;
}

void D3DBuffer::unlock() {
}
