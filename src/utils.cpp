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
#include <cstdlib>
#include <cstring>
#include "log.h"
#include "utils.h"

namespace IPS {

/**
 * Create a copy of the source file.
 * @param [in] sourceFilename Source filename.
 * @param [in] destFilename   Destination filename.
 * @return File descriptor pointing to the beginnig of the destination
 *         file or @b nullptr if something went wrong.
 */
FILE* copyFile(std::string const& sourceFilename, std::string const& destFilename)
{
    FILE *input;
    FILE *output;
    
    input = fopen(sourceFilename.c_str(), "rb");
    if(nullptr == input)
    {
        Error("Failed to open %s : %s", sourceFilename.c_str(), strerror(errno));
        return nullptr;
    }

    output = fopen(destFilename.c_str(), "wb");
    if(nullptr == output)
    {
        Error("Failed to open %s : %s", destFilename.c_str(), strerror(errno));
    }
    else
    {
        size_t n;
        bool ret = true;
    
        while(!feof(input) && ret)
        {
            uint8_t byte;
            n = fread(&byte, 1, 1, input);
            if(1 == n)
            {
                n = fwrite(&byte, 1, 1, output);
                if(1 != n)
                {
                    Error("Failed to write data to %s : %s", destFilename.c_str(), strerror(errno));
                    ret = false;
                }
            }
            else if(!feof(input))
            {
                Error("Failed to read data from %s: %s", sourceFilename.c_str(), strerror(errno));
                ret = false;
            }
        }
        fseek(output, 0, SEEK_SET);
        
        if(false == ret)
        {
            fclose(output);
            output = nullptr;
        }
    }

    fclose(input);
    return output;
}

/**
 * Apply patch to input file and write output to another file.
 * @param [in] in      Input filename.
 * @param [in] out     Output filename.
 * @param [in] patch   IPS patch.
 * @param [in] verbose Output informations. 
 */
bool apply(const char* in, const char* out, IPS::Patch const& patch, bool verbose)
{
    FILE *output;
    output = IPS::copyFile(in, out);    
    if(nullptr == output)
    {
        return false;
    }
    
    // Get output length.
    size_t outputLength;
    fseek(output, 0, SEEK_END);
    outputLength  = ftell(output);
    fseek(output, 0, SEEK_SET);
    outputLength -= ftell(output);
    
    // Write records.
    size_t n;
    bool ret = true;
    for(size_t i=0; ret && (i<patch.count()); i++)
    {
        IPS::Record const& record = patch[i];
        if(verbose)
        {
            Info("Applying record: %5d    offset: %08x    size: %5d    rle: %s",
                 i, record.offset, record.size, record.rle ? "yes" : "no");
        }
        
        // If the offset is beyond output, fill with empty byte.
        if(outputLength < record.offset)
        {
            while(ret && (outputLength < record.offset))
            {
                uint8_t byte = 0x00;
                n = fwrite(&byte, 1, 1, output);
                if(1 != n)
                {
                    Error("Failed to write record #%d: %s", i, strerror(errno));
                    ret = false;
                }
                outputLength += n;
            }
            if(verbose)
            {
                Info("Applying record: filled %d bytes", record.offset - outputLength);
            }
            outputLength += record.size;
        }
        else
        {
            fseek(output, record.offset, SEEK_SET);
        }
        
        if(record.rle)
        {
            uint8_t byte = static_cast<uint8_t>(record.data);
            for(size_t j=0; ret && (j<record.size); j++)
            {
                n = fwrite(&byte, 1, 1, output);
                if(1 != n)
                {
                    Error("Failed to write record #%d: %s", i, strerror(errno));
                    ret = false;
                }
            }
        }
        else
        {
            n = fwrite(reinterpret_cast<uint8_t*>(record.data), 1, record.size, output);
            if(record.size != n)
            {
                Error("Failed to write record #%d: %s", i, strerror(errno));
                ret = false;
            }
        }
    }
    fclose(output);
    return ret;
}


} // namespace IPS

