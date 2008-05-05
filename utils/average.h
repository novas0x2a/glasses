#ifndef AVERAGE_H
#define AVERAGE_H

#include <iostream>
#include <list>

namespace novas0x2a
{
    // An interface for defining a running calculation. Provides caching.
    template <typename T>
    class RunningCalc
    {
        public:
            virtual void reset()          = 0;

             // Adder function with caching
            virtual void add(T data);

             // Getter function with caching
            virtual    T get()      const;

        protected:
            // children implement these
            virtual void add_real(T data) = 0;
            virtual    T get_real() const = 0;

        private:
            mutable T cache;
            mutable bool cache_valid;
    };

    template <typename T>
    class RunningAverage : public RunningCalc<T>
    {
        public:
             // Keep a sliding window of n samples
            explicit RunningAverage(uint32_t samples) : samples(samples) {};

            // Flush the data
            void reset();

            // Actually add the data to the set
            void add_real(T data);

            // Get the data
            T get_real() const;
        private:
            uint32_t samples;
            std::list<T> dataset;
    };

    template <typename T>
    T RunningCalc<T>::get() const
    {
        if (cache_valid)
            return cache;
        else
        {
            cache_valid = true;
            cache = get_real();
            return cache;
        }
    }
    template <typename T>
    void RunningCalc<T>::add(T data)
    {
        cache_valid = false;
        add_real(data);
    }


    template <typename T>
    void RunningAverage<T>::reset()
    {
        dataset.clear();
    }

    template <typename T>
    void RunningAverage<T>::add_real(T data)
    {
        dataset.push_front(data);
        dataset.resize(samples);
    }

    template <typename T>
    T RunningAverage<T>::get_real() const
    {
        T sum = 0;
        typename std::list<T>::const_iterator i,end;
        for (i = dataset.begin(), end = dataset.end(); i != end; ++i)
            sum += *i;
        return sum / samples;
    }
}

#endif
