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
#include <algorithm>
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

/**
 * Default constructor.
 */
Patch::Patch()
    : _records()
{}
/**
 * Destructor.
 */
Patch::~Patch()
{}
/**
 * Add record to patch.
 * @param [in] record  Record 
 * @return @b false if the record overlaps the ones already
 *         stored in the patch.
 */
bool Patch::add(Record const& record)
{
    if(false == _records.empty())
    {
        // Find the right spot.
        int m0 = 2*record.offset + record.size;
        for(size_t i=0; i<_records.size(); i++)
        {
            int m1 = 2*_records[i].offset + _records[i].size;
            int w  = record.size + _records[i].size;
            bool overlap =  (abs(m0 - m1) <= w);
            if(overlap)
            {
                return false;
            }
            if(_records[i].offset > record.offset)
            {
                _records.insert(_records.begin()+i, record);
                return true;
            }
        }
    } 
    _records.push_back(record);
    return true;
}

/**
 * Remove record from patch.
 * @param [in] record  Record to be removed.
 * @return @b false if the record is not in the patch.
 */
bool Patch::remove(Record const& record)
{
    // [todo]
    return true;
}
/**
 * Returns the number of record in the patch.
 */
size_t Patch::count() const
{
    return _records.size();
}
/**
 * Access the record at the index @b i .
 */
Record& Patch::operator[] (size_t i)
{
    return _records[i];
}
/**
 * Access the record at the index @b i .
 */
Record const& Patch::operator[] (size_t i) const
{
    return _records[i];
}

} // namespace IPS
