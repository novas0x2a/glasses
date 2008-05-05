// Ideas and most of the code in this file from Paludis by Ciaran McCreesh,
// et al. Used under the terms of the GPL v2
#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <exception>
#include "stringify.h"

namespace novas0x2a
{
    class Context
    {
        public:
            Context(const std::string &);
            ~Context();
            static std::string backtrace(const std::string &delim);

        private:
            Context(const Context &);
            const Context & operator= (const Context &);
    };

    class Exception : public std::exception
    {
        private:
            const std::string _message;
            mutable std::string _what_str;
            struct ContextData;
            ContextData* const _context_data;
            const Exception & operator= (const Exception &);

        protected:
            Exception(const std::string & message) throw ();
            Exception(const Exception &);

        public:
            virtual ~Exception() throw ();
            const std::string & message() const throw();
            std::string backtrace(const std::string & delim) const;
            bool empty() const;
            const char * what() const throw ();
    };

    // The string identifies the problem
    class GeneralError : public Exception
    {
        public:
            GeneralError(const std::string& location, const std::string& our_message) throw ():
                Exception(location + ": " + our_message) {};
    };

    // A bad argument was passed somewhere
    class CommandLineError : public Exception
    {
        public:
            CommandLineError(const std::string& our_message) throw ():
                Exception(our_message) {};
    };

    // A bad argument was passed somewhere
    class ArgumentError : public Exception
    {
        public:
            ArgumentError(const std::string& our_message) throw ():
                Exception(our_message) {};
    };

    // A bad argument was passed somewhere
    class UnimplementedError : public Exception
    {
        public:
            UnimplementedError(const std::string& our_message) throw ():
                Exception(std::string("Unimplemented: ") + our_message) {};
    };

    // Implemented in cc to keep sdl out of the header
    class SDLError : public Exception
    {
        public:
            SDLError(const std::string& our_message) throw ();
    };

    // Implemented in cc to keep sdl out of the header
    class TTFError : public Exception
    {
        public:
            TTFError(const std::string& our_message) throw ();
    };

#define DEBUG_HERE (std::string("In [") + __PRETTY_FUNCTION__ + "]\n\tat " + \
        std::string(__FILE__) + ":" + stringify(__LINE__))
#define FUNCTION_HERE __PRETTY_FUNCTION__
}

#endif
