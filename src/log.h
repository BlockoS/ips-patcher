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
#ifndef _LOG_H_
#define _LOG_H_

#include <string>

namespace Log {
/**
 * Log type.
 */
struct Type
{
    /** Log type values. **/
    enum Value
    {
        Info,       /**< Information. **/
        Warning,    /**< Warning. **/
        Error       /**< Error. **/
    };
    Value value; /**< Type value. **/
    /** Default constructor. **/
    inline Type() { /* nothing */ }
    /** 
     * Constructor.
     * @param [in] t Type.
     */
    inline Type(Value v) : value(v) {}
    inline operator Value() { return value; }
    inline operator Value() const { return value; }
    /**
     * Type name.
     * @return Type as string. 
     */
    const char* name() const;
};
/**
 * Log string output.
 */
class Output
{
    public:
        /** Default constructor. **/
        Output();
        /** Destructor. **/
        virtual ~Output();
        /**
         * Output log string.
         * @param [in] type    Log type.
         * @param [in] format  Format string.
         * @param [in] arg     Format arguments.
         */
        virtual void out(Type type, const char* format, va_list args);
};
/**
 * Logger (evil singleton).
 */
class Logger
{
    public:
        /** Destructor. **/
        ~Logger();
        /**
         * Begin logger.
         * @param [in] out  Log output.
         */
        void begin(Output *out);
        /**
         * Stop logger.
         */
        void end();
        /**
         * Output log string.
         * @param [in] type    Log type.
         * @param [in] format  Format string.
         * @param [in] ...     Format parameters.
         */
        void out(Type type, const char* format, ...);
        /** Get logger instance. */
        static Logger& instance();
    private:
        /** Default constructor. **/
        Logger();
        /** Constructor. **/
        Logger(Logger const&);
        /** Copy operator. **/
        Logger& operator= (Logger const&);
    private:
        Output* _output;
};

} // namespace Log


#define Info(format, ...)    do { Log::Logger::instance().out(Log::Type::Info,    format, ##__VA_ARGS__); } while(0);
#define Warning(format, ...) do { Log::Logger::instance().out(Log::Type::Warning, format, ##__VA_ARGS__); } while(0);
#define Error(format, ...)   do { Log::Logger::instance().out(Log::Type::Error,   format, ##__VA_ARGS__); } while(0);

#endif /* _LOG_H_ */
