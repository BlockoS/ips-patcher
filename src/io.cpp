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
#include <cstdio>
#include <cstring>
#include <errno.h>
#include "log.h"
#include "io.h"

namespace IPS {

const char* IO::Header = "PATCH";
const off_t IO::HeaderSize = 5;

const char* IO::Footer = "EOF";
const off_t IO::FooterSize = 3;

/** Default constructor. **/
IO::IO()
    : _stream(nullptr)
    , _filename("(none)")
    , _offset(0)
{}
/** Destructor. **/
IO::~IO()
{
    if(nullptr != _stream)
    {
        fclose(_stream);
    }
}
/**
 * Read header and check its validity.
 */
bool IO::readHeader()
{
    uint8_t buffer[5];
    size_t nRead;

    nRead = fread(buffer, 1, IO::HeaderSize, _stream);
    if(IO::HeaderSize != (off_t)nRead)
    {
        Error("Failed to read header");
        return false;
    }
    
    if(memcmp(IO::Header, buffer, IO::HeaderSize))
    {
        Error("Invalid header");
        return false;
    }
    
    return true;
}
/**
 * Read footer and check its validity.
 */
bool IO::readFooter()
{
    char buffer[3];
    size_t nRead;

    // Jump to the end of file and look for the footer.
    fseek(_stream, -IO::FooterSize, SEEK_END);
    
    nRead = fread(buffer, 1, IO::FooterSize, _stream);
    if(IO::FooterSize != (off_t)nRead)
    {
        Error("Failed to read footer: %s", strerror(errno));
        return false;
    }
    
    if(memcmp(IO::Footer, buffer, IO::FooterSize))
    {
        Error("Invalid footer");
        return false;
    }
    
    return true;
}
/** 
 * Read record.
 * @param [out] record IPS record.
 * @return @b true on success.
 */
bool IO::readRecord(Record& record)
{
    uint8_t buffer[5];
    size_t nRead;
    
    // Read offset and size.
    nRead = fread(buffer, 1, 5, _stream);
    _offset += nRead;
    if(5 != nRead)
    {
        Error("Failed to read record offset and size: %s", strerror(errno));
        return false;
    }
    
    record.offset = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
    record.size   = (buffer[3] <<  8) | buffer[4];
    
    // Check for RLE record.
    if(0 == record.size)
    {
        record.rle = true;
        // Read rle size and byte
        nRead = fread(buffer, 1, 3, _stream);
        _offset += nRead;
        if(3 != nRead)
        {
            Error("Failed to read RLE data: %s", strerror(errno));
            return false;
        }
        record.size = (buffer[0] << 8) | buffer[1];
        record.data = static_cast<uintptr_t>(buffer[2]);
    }
    else
    {
        record.rle = false;
        // Read data
        uint8_t* data = new uint8_t[record.size];
        if(nullptr == data)
        {
            Error("Failed to allocate %ud bytes: %s", record.size, strerror(errno));
            return false;
        }
        nRead = fread(data, 1, record.size, _stream);
        _offset += nRead;
        if(record.size != nRead)
        {
            Error("Failed to read data: %s", strerror(errno));
            return false;
        }
        record.data = reinterpret_cast<uintptr_t>(data);
    }

    return true;
}
/**
 * Internal implementation of IPS patch reading.
 */
bool IO::readImpl(Patch& patch)
{
    bool ret;
    
    ret = readHeader();
    if(false == ret) { return ret; }

    ret = readFooter();
    if(false == ret) { return ret; }

    // Compute data size
    size_t len;
    fseek(_stream, -IO::FooterSize, SEEK_END);
    len  = ftell(_stream);
    fseek(_stream,  IO::HeaderSize, SEEK_SET);
    len -= ftell(_stream);
    
    for(_offset=0; ret && (_offset<len); )
    {
        Record record;
        ret = readRecord(record);
        if(ret)
        {
            ret = patch.add(record);
            if(false == ret)
            {
                Error("Failed to add record.");
            }
        }
    }

    return ret;
}
/**
 * Read IPS patch.
 * @param [in]  filename IPS patch filename.
 * @param [out] patch IPS patch.
 */
bool IO::read(std::string const& filename, Patch& patch)
{
    _stream = fopen(filename.c_str(), "rb");
    if(nullptr == _stream)
    {
        Error("Failed to open %s: %s", filename.c_str(), strerror(errno));
        return false;
    }
    
    bool ret = readImpl(patch);
    
    fclose(_stream);
    _stream = nullptr;
    
    return ret;
}
/**
 * Internal implementation of IPS patch writing.
 */
bool IO::writeImpl(Patch const& patch)
{
    uint8_t buffer[8];
    size_t nWritten;
    _offset = 0;
    // Write header.
    nWritten = fwrite(IO::Header, 1, IO::HeaderSize, _stream);
    _offset += nWritten;
    if(IO::HeaderSize != (off_t)nWritten)
    {
        Error("Failed to write header : %s", strerror(errno));
        return false;
    }
    // Write records.
    for(size_t i=0; i<patch.count(); i++)
    {
        Record const& record = patch[i];
        // Offset.
        buffer[0] = (record.offset >> 16) & 0xff;
        buffer[1] = (record.offset >>  8) & 0xff;
        buffer[2] = (record.offset      ) & 0xff;
        if(false == record.rle)
        {
            // Size.
            buffer[3] = (record.size >> 8) & 0xff;
            buffer[4] = (record.size     ) & 0xff;
            // Write header.
            nWritten = fwrite(buffer, 1, 5, _stream);
            _offset += nWritten;
            if(5 != nWritten)
            {
                Error("Failed to write record #%d header: %s", i, strerror(errno));
                return false;
            }
            // Data.
            nWritten = fwrite(reinterpret_cast<uint8_t*>(record.data), 1, record.size, _stream);
            _offset += nWritten;
            if(record.size != nWritten)
            {
                Error("Failed to write record #%d data: %s", i, strerror(errno));
                return false;
            }
        }
        else
        {
            // Size.
            buffer[3] = 0;
            buffer[4] = 0;
            // Record data is 3 bytes long.
            // -- 1st and 2nd bytes are repeat count
            buffer[5] = (record.size >> 8) & 0xff;
            buffer[6] = (record.size     ) & 0xff;
            // -- 3rd byte is the repeated data
            buffer[7] = static_cast<uint8_t>(record.data & 0xff);
            // Write RLE record.
            nWritten = fwrite(buffer, 1, 8, _stream);
            _offset += nWritten;
            if(8 != nWritten)
            {
                Error("Failed to write RLE record %d: %s", i, strerror(errno));
                return false;
            }
        }
    }
    // Write footer.
    nWritten = fwrite(IO::Footer, 1, IO::FooterSize, _stream);
    _offset += nWritten;
    if(IO::FooterSize != (off_t)nWritten)
    {
        Error("Failed to write footer : %s", strerror(errno));
        return false;
    }
    return true;
}
/**
 * Write IPS patch.
 * @param [in] filename IPS patch filename.
 * @param [in] patch    IPS patch.
 */
bool IO::write(std::string const& filename, Patch const& patch)
{
    _stream = fopen(filename.c_str(), "wb");
    if(nullptr == _stream)
    {
        Error("Failed to open %s: %s", filename.c_str(), strerror(errno));
        return false;
    }
    
    bool ret = writeImpl(patch);
    
    fclose(_stream);
    _stream = nullptr;
    
    return ret;
}

} // namespace IPS
