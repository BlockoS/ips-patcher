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
#ifndef _IPS_H_
#define _IPS_H_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace IPS {

/**
 * IPS result values.
 */ 
enum Result
{
    IPS_ERROR_INVALID   = -9,
    IPS_ERROR_OFFSET    = -8,
    IPS_ERROR_PROCESS   = -7,
    IPS_ERROR_READ      = -6,
    IPS_ERROR_WRITE     = -5,
    IPS_ERROR_SAVE      = -4,
    IPS_ERROR_OPEN      = -3,
    IPS_ERROR_FILE_TYPE = -2,
    IPS_ERROR           = -1,
    IPS_PATCH_END       =  0,
    IPS_OK              =  1
};

/**
 * IPS record.
 */
struct Record
{
    bool      rle;    /**< If not zero, the record is RLE encoded. */
    uintptr_t data;   /**< Data pointer or RLE data. */
    uint32_t  offset; /**< Destination offset. */
    uint16_t  size;   /**< Data size. */
    
    /**
     * Default constructor.
     */
    Record();
    /**
     * Creates a standard record.
     * @param [in]  offset  Destination offset.
     * @param [in]  size    Data size.
     * @param [in]  pointer Pointer to source data.
     */
    Record(uint32_t offset, uint16_t size, uintptr_t pointer);

    /**
     * Creates a RLE encoded record.
     * @param [out] record IPS record.
     * @param [in]  offset Destination offset.
     * @param [in]  size   Data size (the number of time the data byte 
     *                     will be repeated in the destination data).
     * @param [in]  data   Data byte.
     */
    Record(uint32_t offset, uint16_t size, uint8_t data);
};

/**
 * An IPS patch is basically a list of records.
 */
class Patch
{
    public:
        /**
         * Default constructor.
         */
        Patch();
        /**
         * Destructor.
         */
        ~Patch();
        /**
         * Add record to patch.
         * @param [in] record  Record 
         * @param [in] check   If @b true check if records overlap.
         * @return @b false if the record overlaps the ones already
         *         stored in the patch.
         */
        bool add(Record const& record, bool check=true);
        // [todo] get overlapping records
        /**
         * Remove the record at b index.
         * @param [in] index  Record index.
         * @return @b false if the index is out of bound.
         */
        bool remove(size_t index);
        /**
         * Returns the number of record in the patch.
         */
        size_t count() const;
        /**
         * Access the record at the index @b i .
         */
        Record& operator[] (size_t i);
        /**
         * Access the record at the index @b i .
         */
        Record const& operator[] (size_t i) const;        
    private:
        /** Record array. **/
        std::vector<Record> _records;
};

} // namespace IPS

#endif /* _IPS_H_ */
