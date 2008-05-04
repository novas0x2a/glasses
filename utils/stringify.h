#ifndef STRINGIFY_H
#define STRINGIFY_H

#include <sstream>
#include <string>

namespace novas0x2a
{
    namespace stringify_internals
    {
        template <typename T_>
        struct CheckType
        {
            // Yes, we are a sane type.
            enum { value = 0 } Value;
        };

        /**
         * Check that T_ is a sane type to be stringified, which it isn't
         * if it's a pointer unless it's a char * pointer.
         */
        template <typename T_>
        struct CheckType<T_ *>
        {
        };

        /**
         * Check that T_ is a sane type to be stringified, which it isn't
         * if it's a pointer unless it's a char * pointer.
         */
        template <>
        struct CheckType<char *>
        {
            /// Yes, we are a sane type.
            enum { value = 0 } Value;
        };
    }

    template <typename T_> inline std::string stringify(const T_ & item);

    namespace stringify_internals
    {
        /**
         * Internal function to convert item to a string, to make
         * function pointers work more sensibly.  May be overloaded,
         * but should not be called directly.
         */
        template <typename T_>
        std::string
        real_stringify(const T_ & item)
        {
            std::ostringstream s;
            s << item;
            return s.str();
        }

        inline std::string
        real_stringify(const std::string & item)
        {
            return item;
        }

        inline std::string
        real_stringify(const char & item)
        {
            return std::string(1, item);
        }

        inline std::string
        real_stringify(const unsigned char & item)
        {
            return std::string(1, item);
        }

        inline std::string
        real_stringify(const bool & item)
        {
            return item ? "true" : "false";
        }

        inline std::string
        real_stringify(const char * const item)
        {
            return std::string(item);
        }
    }

    /**
     * Convert item to a string.  To customise for new types, overload
     * stringify_internals::real_stringify, not this function.
     */
    template <typename T_>
    inline std::string
    stringify(const T_ & item)
    {
        return stringify_internals::real_stringify(item);
    }
}

#endif
