#ifndef GLOBAL_H
#define GLOBAL_H

#include "utils/context.h"

// These affect how gcc lays out branches
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

// The attribute makes it so gcc makes better assumptions about optimization
typedef unsigned char byte;
typedef byte Pixel __attribute__((vector_size (4)));

// BGRA
inline byte&  R(const Pixel &p) {return ((byte*)&p)[2];}
inline byte&  G(const Pixel &p) {return ((byte*)&p)[1];}
inline byte&  B(const Pixel &p) {return ((byte*)&p)[0];}
inline byte&  A(const Pixel &p) {return ((byte*)&p)[3];}
inline byte   Vb(const Pixel &p) {return 0.2989*R(p) + 0.5866*G(p) + 0.1145*B(p);}
inline double Vd(const Pixel &p) {return 0.2989*R(p) + 0.5866*G(p) + 0.1145*B(p);}

inline Pixel RGB(byte r, byte g, byte b, byte a = 1) {return (Pixel){b,g,r,a};}

// TODO: Bleh. Config file.
#define FONT "Vera.ttf"

#endif
