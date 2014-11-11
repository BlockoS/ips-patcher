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
#ifndef _IPS_UTILS_H_
#define _IPS_UTILS_H_

#include <string>
#include <cstdio>
#include "ips.h"

namespace IPS {
/**
 * Create a copy of the source file.
 * @param [in] sourceFilename Source filename.
 * @param [in] destFilename   Destination filename.
 * @return File descriptor pointing to the beginnig of the destination
 *         file or @b nullptr if something went wrong.
 */
FILE* copyFile(std::string const& sourceFilename, std::string const& destFilename);
/**
 * Apply patch to input file and write output to another file.
 * @param [in] in      Input filename.
 * @param [in] out     Output filename.
 * @param [in] patch   IPS patch.
 * @param [in] verbose Output informations. 
 */
bool apply(const char* in, const char* out, IPS::Patch const& patch, bool verbose);

} // namespace IPS

#endif /* _IPS_UTILS_H_ */
