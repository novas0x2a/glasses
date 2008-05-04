#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include <list>

namespace novas0x2a
{
    template <typename T>
    class RunningCalc
    {
        public:
            virtual void reset()     = 0;
            virtual void add(T data) = 0;
            virtual    T get() const = 0;
    };

    template <typename T>
    class RunningAverage : public RunningCalc<T>
    {
        public:
            explicit RunningAverage(uint32_t samples) : samples(samples) {};
            void reset();
            void add(T data);
               T get() const;
        private:
            uint32_t samples;
            std::list<T> dataset;
    };

    template <typename T>
    void RunningAverage<T>::reset()
    {
        dataset.clear();
    }

    template <typename T>
    void RunningAverage<T>::add(T data)
    {
        dataset.push_front(data);
        dataset.resize(samples);
    }

    template <typename T>
    T RunningAverage<T>::get() const
    {
        T sum = 0;
        typename std::list<T>::const_iterator i,end;
        for (i = dataset.begin(), end = dataset.end(); i != end; ++i)
            sum += *i;
        return sum / samples;
    }
}

#endif
