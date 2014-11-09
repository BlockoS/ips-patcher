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
#ifndef _IPS_IO_H_
#define _IPS_IO_H_

#include <string>
#include "ips.h"

namespace IPS {

/**
 * IPS patch input/output.
 */
class IO
{
    public:
        static const char* Header;
        static const off_t HeaderSize;
        static const char* Footer;
        static const off_t FooterSize;
        
    public:
        /** Default constructor. **/
        IO();
        /** Destructor. **/
        ~IO();
        /**
         * Read IPS patch.
         * @param [in]  filename IPS patch filename.
         * @param [out] patch    IPS patch.
         */
        bool read(std::string const& filename, Patch& patch);
        /**
         * Write IPS patch.
         * @param [in] filename IPS patch filename.
         * @param [in] patch    IPS patch.
         */
        bool write(std::string const& filename, Patch const& patch);

    private:
        /** Read header and check its validity. **/
        bool readHeader();
        /** Read footer and check its validity. **/
        bool readFooter();
        /** 
         * Read record.
         * @param [out] record IPS record.
         * @return @b true on success.
         */
        bool readRecord(Record& record);
        /**
         * Internal implementation of IPS patch reading.
         */
        bool readImpl(Patch& patch);
        /**
         * Internal implementation of IPS patch writing.
         */
        bool writeImpl(Patch const& patch);
        
    private:
        /** File handle. **/
        FILE* _stream;
        /** Filename. **/
        std::string _filename;
        /** File offset. **/
        size_t _offset;
};

} // namespace IPS

#endif /* _IPS_IO_H_ */
