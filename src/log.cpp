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
#include <cstdarg>
#include "log.h"

namespace Log {
/**
 * Type name.
 * @return Type as string. 
 */
const char* Type::name() const
{
    switch(value)
    {
        case Type::Info:
            return "info";
        case Type::Warning:
            return "warning";
        case Type::Error:
            return "error";
        default:
            return "unknown";
    }
}
/** Default constructor. **/
Output::Output()
{}
/** Destructor. **/
Output::~Output()
{}
/**
 * Output log string.
 * @param [in] type    Log type.
 * @param [in] format  Format string.
 * @param [in] arg     Format arguments.
 */
void Output::out(Type type, const char* format, va_list args)
{
    fprintf(stderr, "[%s] ", type.name());
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

/** Destructor. **/
Logger::~Logger()
{}
/**
* Begin logger.
* @param [in] out  Log output.
*/
void Logger::begin(Output *out)
{
    _output = out;
}
/**
* Stop logger.
*/
void Logger::end()
{
}
/**
* Output log string.
* @param [in] type    Log type.
* @param [in] format  Format string.
* @param [in] ...     Format parameters.
*/
void Logger::out(Type type, const char* format, ...)
{
    if(nullptr == _output)
    {
        return;
    }
    va_list args;
    va_start(args, format);
    _output->out(type, format, args);
    va_end(args);
}
/** Get logger instance. */
Logger& Logger::instance()
{
    static Logger loggerInstance;
    return loggerInstance;
}
/** Default constructor. **/
Logger::Logger()
    : _output(nullptr)
{}
/** Constructor. **/
Logger::Logger(Logger const&)
    : _output(nullptr)
{}
/** Copy operator. **/
Logger& Logger::operator= (Logger const&)
{
    return *this;
}

} // namespace Log
