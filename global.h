#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <sstream>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

typedef unsigned char byte;
typedef byte Pixel __attribute__((vector_size (4)));

inline byte& R(const Pixel &p) {return ((byte*)&p)[2];}
inline byte& G(const Pixel &p) {return ((byte*)&p)[1];}
inline byte& B(const Pixel &p) {return ((byte*)&p)[0];}
inline byte& A(const Pixel &p) {return ((byte*)&p)[3];}

inline Pixel RGB(byte r, byte g, byte b, byte a = 0) {return (Pixel){b,g,r,a};}

template<typename T>
inline std::string stringify(const T& x)
{
    std::ostringstream o;
    o << x;
    return o.str();
}

#endif
