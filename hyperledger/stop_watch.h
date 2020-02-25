//
// Created by hama on 20. 2. 25..
//

#ifndef HYPERLEDGER_FABRIC_400_CPP_STOP_WATCH_H
#define HYPERLEDGER_FABRIC_400_CPP_STOP_WATCH_H

#endif //HYPERLEDGER_FABRIC_400_CPP_STOP_WATCH_H

#include <iostream>
#include <chrono>

#include "string_format.h"

namespace hama {

    class StopWatch {

    public:
        StopWatch() : _beg(Clock::now()) {

            _total = 0.0;
        }

        void reset() {

            _beg = Clock::now();
        }

        std::string check() {

            double t = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _beg).count() / 1000.0;
            _total += t;
            reset();
            return string_format(" %f sec", t);
        }

        std::string stop() {

            double t = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _beg).count() / 1000.0;
            _total += t;
            return string_format(" %f sec", _total);
        }

    private:
        typedef std::chrono::high_resolution_clock Clock;
        typedef std::chrono::duration<double, std::ratio<1> > Second;

        std::chrono::time_point<Clock> _beg;
        double _total = 0.0;
    };
}

