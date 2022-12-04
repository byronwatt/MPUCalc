/*******************************************************************************
*   COPYRIGHT (C) 2016 MICROSEMI, INC. ALL RIGHTS RESERVED.
* --------------------------------------------------------------------------
*  This software embodies materials and concepts which are proprietary and
*  confidential to Microsemi, Inc.
*  Microsemi distributes this software to its customers pursuant to the
*  terms and conditions of the Software License Agreement
*  contained in the text file software.lic that is distributed along with
*  the software. This software can only be utilized if all
*  terms and conditions of the Software License Agreement are
*  accepted. If there are any questions, concerns, or if the
*  Software License Agreement text file, software.lic, is missing please
*  contact Microsemi for assistance.
* --------------------------------------------------------------------------
*/
/**
* @file
* @brief
*   This file defines the DisjointRangeVector class
*   which flattens a set of overlapping ranges into a disjoint set of intervals.
*
* For example if we have the following four ranges:
*
*      range[0] = 0..10 A
*      range[1] = 2..10 B
*      range[2] = 6..12 C
*      range[3] = 20..30 D
*
* This could be shown as a table like:
*
*         0   A                  
*         1   A                  
*         2   A B                
*         3   A B                
*         4   A B                
*         5   A B                
*         6   A B C              
*         7   A B C              
*         8   A B C              
*         9   A B C              
*        10   A B C              
*        11       C              
*        12       C              
*        13                      
*        14                      
*        15                      
*        16                      
*        17                      
*        18                      
*        19                      
*        20        D             
*        21        D             
*        22        D             
*        23        D             
*        24        D             
*        25        D             
*        26        D             
*        27        D             
*        28        D             
*        29        D             
*        30        D             
*
* You can think of it as a Venn diagram.
*   some points are contained in range A
*   some points are contained in range A and B
*   some points are contained in range A, B and C
*   some points are contained in just range C
*   some points are contained in just range D
*   and some points do not overlap with the given ranges.
*
* The DisjointRangeVector stores this as a list of disjoint ranges like:
*
*     [0] RangeList(-100, -1): 
*     
*     [1] RangeList(0, 1): 
*         [0] RangeValue(0, 10): A
*     
*     [2] RangeList(2, 5): 
*         [0] RangeValue(0, 10): A
*         [1] RangeValue(2, 10): B
*     
*     [3] RangeList(6, 10): 
*         [0] RangeValue(0, 10): A
*         [1] RangeValue(2, 10): B
*         [2] RangeValue(6, 12): C
*     
*     [4] RangeList(11, 12): 
*         [0] RangeValue(6, 12): C
*     
*     [5] RangeList(13, 19): 
*     
*     [6] RangeList(20, 30): 
*         [0] RangeValue(20, 30): D
*     
*     [7] RangeList(31, 100): 
*
* And with O(lg N), the DisjointRangeVector class return the list of ranges that overlap with a paricular point,
* and also returns the extent of the interval with the same set of ranges.
*
* see range_unit_test.c for the unit test
*/
#ifndef __DISJOINT_RANGE_VECTOR_H
#define __DISJOINT_RANGE_VECTOR_H

#include <vector>
#include <list>
#include <algorithm>
#ifndef MDX2_SMALL_MEMORY
#include <iostream>
#include <iomanip>
#endif
#include <memory>
#include <cassert>

/// enable debugging for this module.
#define RANGE_DEBUG(...)
//#define RANGE_DEBUG(...) std::cout << __VA_ARGS__ << std::endl

// a RangeValue is a start,stop and object
template <class Scalar, typename Value>
/// a range and a value
class RangeValue {
public:
    Scalar start; ///< start value
    Scalar stop; ///< stop value
    Value value; ///< value
    /// constructor
    RangeValue(const Scalar& s, const Scalar& e, const Value& v)
    : start(std::min(s, e))
    , stop(std::max(s, e))
    , value(v) 
    {}
};

#ifndef MDX2_SMALL_MEMORY
template <class Scalar, typename Value>
/// ostream operator for RangeValue
std::ostream& operator<<(std::ostream& out, const RangeValue<Scalar, Value>& rv) {
    if ((rv.start > 1000) || (rv.stop > 1000))
    {
        std::ios_base::fmtflags f( out.flags() );
        out << std::setfill('0') << std::setw(8) << std::hex;
        out << "RangeValue(0x" << rv.start << ", 0x" << rv.stop << "): " ;
        out.flags( f );
        out << rv.value;
    }
    else
    {
        out << "RangeValue(" << rv.start << ", " << rv.stop << "): " << rv.value;
    }
    return out;
}
#endif

template <class Scalar, class Value>
/// a DisjointInterval is used to indicate one disjoint interval containing a set of ranges that overlap with this interval.
class DisjointInterval {
public:
    Scalar start; ///< start of interval
    Scalar stop;  ///< stop of interval
    std::list< RangeValue<Scalar,Value> > _list; ///< ranges that overlap with this interval
    /// default constructor
    DisjointInterval()
    : start()
    , stop()
    , _list() 
    {}
    /// constructor
    DisjointInterval(const Scalar& s, const Scalar& e)
    : start(std::min(s, e))
    , stop(std::max(s, e))
    , _list() 
    {}
    /// returns true if there are no ranges in this interval.
    bool empty() const {return _list.empty();}
};

#ifndef MDX2_SMALL_MEMORY
template <class Scalar, typename Value>
/// ostream operator for DisjointInterval
std::ostream& operator<<(std::ostream& out, const DisjointInterval<Scalar, Value>& rl) {
    if (rl.start > 1000)
    {
        std::ios_base::fmtflags f( out.flags() );
        out << std::setfill('0') << std::setw(8) << std::hex;
        out << "DisjointInterval(0x" << rl.start << ", 0x" << rl.stop << "): " << std::endl;
        out.flags( f );
    }
    else
    {
        out << "DisjointInterval(" << rl.start << ", " << rl.stop << "): " << std::endl;
    }

    int count=0;
    for( typename std::list< RangeValue<Scalar,Value> >::const_iterator i=rl._list.begin();
         i != rl._list.end();
         i++)
    {
        out << "    [" << count++ << "] " << *i << std::endl;
    }
    return out;
}

template <class Scalar, typename Value>
/// ostream operator for a vector of RangeValues
std::ostream& operator<<(std::ostream& out, const std::vector< RangeValue<Scalar, Value> > & v) {
    out << "RangeVector:" << std::endl;

    int count=0;
    for (typename std::vector< RangeValue<Scalar, Value> >::const_iterator i = v.begin();
        i != v.end();
        i++)
    {
        out << "[" << count++ << "] " << *i << std::endl;
    }
    return out;
}
#endif

/**
 * class that converts overlapping ranges into a 
 * list of disjoint intervals each of which point to a list of the overlapping ranges.
 *
 * have a list of ranges like:
 * range[0] = 0..10 A
 * range[1] = 2..10 B
 * range[2] = 6..12 C
 * range[3] = 20..30 D
 * 
 * want to generate:
 * 
 * disjoint_range[0] =  0.. 1 : 0..10 A
 * disjoint_range[1] =  2..10 : 0..10 A, 2..10 B
 * disjoint_range[2] =  6..10 : 0..10 A, 2..10 B, 6..12 C
 * disjoint_range[3] = 10..12 : 6..12 C
 * disjoint_range[4] = 13..19 :
 * disjoint_range[5] = 20..30 : 20..30 D
 * disjoint_range[6] = 31..end : 
 */
template <class Scalar, class Value>
class DisjointRangeVector {
public:
    typedef RangeValue<Scalar, Value> range_value; ///< a range and value
    typedef std::vector<range_value> range_vector; ///< a vector of RangeValues
    typedef DisjointInterval<Scalar, Value> disjoint_interval; ///< a disjoint interval and a set of ranges associated with that interval.
    typedef std::list< RangeValue<Scalar,Value> > list_of_ranges; ///< this matches the _list type in DisjointInterval
    typedef std::vector<disjoint_interval> vector_of_disjoint_intervals; ///< a vector of disjoint intervals and their set of ranges.

    /// sort ranges in ascending order
    struct RangeStartCmp {
        /// comparison operator
        bool operator()(const range_value& a, const range_value& b) {
            return a.start < b.start;
        }
    };

    /// destructor
    ~DisjointRangeVector() {}

    /// constructor
    DisjointRangeVector(Scalar start, Scalar stop, range_vector *ivals)
    {
        bool overflow = false;
        _ranges = *ivals;
        // sort _ranges by start
        std::sort(_ranges.begin(),_ranges.end(),RangeStartCmp());
        RANGE_DEBUG( "sorted: " << std::endl << _ranges );
        disjoint_interval active_interval;
        active_interval.start = start;
        if (_ranges.size() == 0)
        {
            active_interval.stop = stop;
            _vector_of_disjoint_intervals.push_back(active_interval);
            RANGE_DEBUG("empty list");
            return;
        }
        for (typename range_vector::iterator i=_ranges.begin();i!=_ranges.end();i++)
        {
            RANGE_DEBUG( "considering: " << *i );
            // if this range also starts at this range....
            if (i->start == active_interval.start)
            {
                RANGE_DEBUG( "appending" );
                active_interval._list.push_back(*i);   
            }
            else
            {
                do {
                    // find smallest 'stop' as either the beginning of the next range,
                    // or the smallest 'stop' in the current range.
                    //
                    active_interval.stop = i->start-1;
                    RANGE_DEBUG( "calculating smallest stop: " << active_interval.stop );
                    for (typename list_of_ranges::iterator j=active_interval._list.begin();j!=active_interval._list.end();j++)
                    {
                        if (j->stop < active_interval.stop)
                        {
                            active_interval.stop = j->stop;
                            RANGE_DEBUG( "new smallest stop: " << active_interval.stop );
                        }
                    }
                    RANGE_DEBUG( "pushing current range: " << active_interval );
                    _vector_of_disjoint_intervals.push_back(active_interval);

                    active_interval.start = active_interval.stop + 1;
                    RANGE_DEBUG( "next start: " << active_interval.start );
                    typename list_of_ranges::iterator x=active_interval._list.begin();
                    while (x != active_interval._list.end())
                    {
                        // remove ranges that do not overlap with start.
                        if (x->stop < active_interval.start)
                        {
                            RANGE_DEBUG( "remove " << *x );
                            x = active_interval._list.erase(x);
                        }
                        else
                        {
                            x++;
                        }
                    }
                } while (active_interval.stop != i->start-1);
                active_interval._list.push_back(*i);
            }
        }
        while (!active_interval._list.empty())
        {
            // find smallest 'stop' as either the beginning of the next range,
            // or the smallest 'stop' in the current range.
            //
            active_interval.stop = active_interval._list.begin()->stop;
            RANGE_DEBUG( "calculating smallest stop: " << active_interval.stop );
            for (typename list_of_ranges::iterator j=active_interval._list.begin();j!=active_interval._list.end();j++)
            {
                if (j->stop < active_interval.stop)
                {
                    active_interval.stop = j->stop;
                    RANGE_DEBUG( "new smallest stop: " << active_interval.stop );
                }
            }
            RANGE_DEBUG( "smallest stop: " << active_interval.stop );
            RANGE_DEBUG( "pushing current range: " << active_interval );
            _vector_of_disjoint_intervals.push_back(active_interval);

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wstrict-overflow"
            active_interval.start = active_interval.stop + 1;
// trying to avoid -Wstrick-overflow problem, but adding the pragma does not work
// since the code is evaluated when instantiated.
//#pragma GCC diagnostic pop
//
// either this strange "it's zero, but you can't trust it" hack,
// or disabling "-Wstrict-overflow" for the entire compilation !
static volatile Scalar zero = 0;
            if (active_interval.start < active_interval.stop + zero)
            {
                // overflow
                overflow = true;
                active_interval.start = active_interval.stop;
                RANGE_DEBUG( "next start (overflow): " << active_interval.start );
                typename list_of_ranges::iterator x=active_interval._list.begin();
                while (x != active_interval._list.end())
                {
                    // remove ranges that do not overlap with start.
                    if (x->stop <= active_interval.start)
                    {
                        RANGE_DEBUG( "remove (overflow)" << *x );
                        x = active_interval._list.erase(x);
                    }
                    else
                    {
                        x++;
                    }
                }

            }
            else
            {
                RANGE_DEBUG( "next start: " << active_interval.start );
                typename list_of_ranges::iterator x=active_interval._list.begin();
                while (x != active_interval._list.end())
                {
                    // remove ranges that do not overlap with start.
                    if (x->stop < active_interval.start)
                    {
                        RANGE_DEBUG( "remove " << *x );
                        x = active_interval._list.erase(x);
                    }
                    else
                    {
                        x++;
                    }
                }
            }
        }
        active_interval.stop = stop;
        if ((active_interval.stop >= active_interval.start) && (!overflow))
        {
            _vector_of_disjoint_intervals.push_back(active_interval);
        }

        RANGE_DEBUG( *this );
    }


    /**
     * @brief
     *   find the disjoint interval that contains the specified element.
     * @param[in] find - element to find.
     * @return pointer to disjoint_interval (which includes a set of ranges that overlap with that interval)
     */
    const disjoint_interval *find_disjoint_interval(Scalar find)
    {
        int min = 0;
        int max = _vector_of_disjoint_intervals.size() - 1;
        int mid = (max + min) / 2;
        RANGE_DEBUG("looking for " << find);
        while (1)
        {
            RANGE_DEBUG("min: "<<min << " max: " << max << " mid: " << mid);
            disjoint_interval *mid_rl = &_vector_of_disjoint_intervals[mid];
            RANGE_DEBUG( "[" << mid << "] " << *mid_rl );
            if (find > mid_rl->stop)
            {
                // if you're already at the max, then return NULL.
                if (mid == max)
                    return NULL;
                min = mid+1;
                mid = (max + min) / 2;
            }
            else if ((find < mid_rl->start))
            {
                // if you're already at the min, then return NULL.
                if (mid == min)
                    return NULL;
                max = mid-1;
                mid = (max + min) / 2;
            }
            else
            {

                return mid_rl;
            }
        }
    }
public:
    range_vector _ranges; ///< overlapping set of ranges
    vector_of_disjoint_intervals _vector_of_disjoint_intervals; ///< vector of disjoint intervals
};

#ifndef MDX2_SMALL_MEMORY
template <class Scalar, typename Value>
/// ostream operator for DisjointRangeVector
std::ostream& operator<<(std::ostream& out, const  DisjointRangeVector<Scalar, Value>& arv) {
    out << "DisjointRangeVector:" << std::endl;

    int count=0;
    for (typename std::vector< DisjointInterval<Scalar, Value> >::const_iterator i = arv._vector_of_disjoint_intervals.begin();
        i != arv._vector_of_disjoint_intervals.end();
        i++)
    {
        out << "[" << count++ << "] " << *i << std::endl;
    }
    return out;
}
#endif

#endif
