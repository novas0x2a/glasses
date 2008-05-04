// Ideas and most of the code in this file from Paludis by Ciaran McCreesh,
// et al. Used under the terms of the GPL v2
#ifndef JOIN_H
#define JOIN_H

#include <iterator>
#include <string>
#include "stringify.h"


namespace novas0x2a
{

    template <typename I_, typename T_>
    T_ join(I_ i, I_ end, const T_ & joiner)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += stringify(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    template <typename I_, typename T_, typename F_>
    T_ join(I_ i, I_ end, const T_ & joiner, const F_ & f)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += (f)(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    template <typename I_>
    std::string join(I_ begin, const I_ end, const char * const t)
    {
        return join(begin, end, std::string(t));
    }

    template <typename I_, typename F_>
    std::string join(I_ begin, const I_ end, const char * const t, const F_ & f)
    {
        return join(begin, end, std::string(t), f);
    }
}

#endif
