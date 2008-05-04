#include <list>
#include <string>
#include <SDL/SDL.h>       // for SDL_GetError
#include <SDL/SDL_ttf.h>   // for TTF_GetError
#include "context.h"
#include "join.h"

using namespace novas0x2a;

namespace
{
    static std::list<std::string> *context = 0;
}

Context::Context(const std::string & s)
{
    if (!context)
        context = new std::list<std::string>;
    context->push_back(s);
}

Context::~Context()
{
    if (!context)
        throw GeneralError(DEBUG_HERE, "no context");
    context->pop_back();
    if (context->empty())
    {
        delete context;
        context = 0;
    }
}

std::string
Context::backtrace(const std::string & delim)
{
    if (!context)
        return "";

    return join(context->begin(), context->end(), delim) + delim;
}

namespace novas0x2a
{
    struct Exception::ContextData
    {
        std::list<std::string> local_context;

        ContextData()
        {
            if (context)
                local_context.assign(context->begin(), context->end());
        }

        ContextData(const ContextData & other) :
            local_context(other.local_context)
        {
        }
    };
}

Exception::Exception(const std::string & m) throw () :
    _message(m),
    _context_data(new ContextData)
{
}

Exception::Exception(const Exception & other) :
    std::exception(other),
    _message(other._message),
    _context_data(new ContextData(*other._context_data))
{
}

Exception::~Exception() throw ()
{
    delete _context_data;
}

const std::string &
Exception::message() const throw ()
{
    return _message;
}

std::string
Exception::backtrace(const std::string & delim) const
{
    return join(_context_data->local_context.begin(), _context_data->local_context.end(), delim) + delim;
}

bool
Exception::empty() const
{
    return _context_data->local_context.empty();
}

SDLError::SDLError(const std::string& our_message) throw ():
    Exception(our_message + " (" + SDL_GetError() + ")") {}

TTFError::TTFError(const std::string& our_message) throw ():
    Exception(our_message + " (" + TTF_GetError() + ")") {}

const char *
Exception::what() const throw ()
{
    if (_what_str.empty())
        _what_str = stringify(std::exception::what());
    return _what_str.c_str();
}
