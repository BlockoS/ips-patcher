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
#include <iostream>
#include <cstring>
#include "log.h"
#include "ips.h"
#include "io.h"
#include "utils.h"

/**
 * Print usage.
 */
void usage()
{
    std::cerr << "usage: ips-patcher-cli source patch destination" << std::endl;
    std::cerr << "       Apply IPS patch to \"source\" file and write output to \"destination\"." << std::endl;
}

/**
 * Main entry point.
 */
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        usage();
        return 0;
    }

    Log::Logger& logger = Log::Logger::instance();
    Log::Output* output = new Log::Output();
    
    IPS::Patch  patch;
    IPS::IO     io;
    bool ret;
    
    logger.begin(output);
    
    ret = io.read(argv[2], patch);
    if(false == ret)
    {
        Error("Failed to read %s", argv[2]);
    }
    else
    {
        apply(argv[1], argv[3], patch, true);
    }
    
    logger.end();

    free(output);

    return 0;
}
