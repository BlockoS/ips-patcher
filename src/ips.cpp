/*
 * IPS Patcher
 * 
 * Copyright (c) 2014, Vincent Cruz, All rights reserved.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */
#include "ips.h"

namespace IPS {

/**
 * Creates a standard record.
 * @param [in]  offset  Destination offset.
 * @param [in]  size    Data size.
 * @param [in]  pointer Pointer to source data.
 */
Record::Record(uint32_t offset, uint16_t size, uintptr_t pointer)
    : rle(false)
    , data(pointer)
    , offset(offset)
    , size(size)
{}

/**
 * Creates a RLE encoded record.
 * @param [out] record IPS record.
 * @param [in]  offset Destination offset.
 * @param [in]  size   Data size (the number of time the data byte 
 *                     will be repeated in the destination data).
 * @param [in]  data   Data byte.
 */
Record::Record(uint32_t offset, uint16_t size, uint8_t data)
    : rle(true)
    , data(static_cast<uintptr_t>(data))
    , offset(offset)
    , size(size)
{}

} // namespace IPS
