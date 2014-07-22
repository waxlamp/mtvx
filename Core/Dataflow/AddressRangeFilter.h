// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// AddressRangeFilter.h - Examines incoming records (which are
// contractually MType (see TODO in [Memory|Line]RecordFilter.h) and
// passes along those that fall either WITHIN or WITHOUT a series of
// address ranges.

#ifndef ADDRESS_RANGE_FILTER_H
#define ADDRESS_RANGE_FILTER_H

// MTV headers.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Filter.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// System headers.
#include <utility> // For std::make_pair().
#include <vector>

namespace MTV{
  template<bool Pass>
  class AddressRangeFilter : public Filter<MTR::Record>,
                             public Filter<CacheAccessRecord>,
                             public Filter<CacheStatusReport> {
  public:
    BoostPointers(AddressRangeFilter);

  private:
    typedef std::vector<std::pair<MTR::addr_t, MTR::addr_t> > range_vector;

  public:
    static typename AddressRangeFilter<Pass>::ptr all(){
      static AddressRangeFilter<Pass>::ptr single;
      if(!single){
        single = boost::make_shared<AddressRangeFilter<Pass> >(0, Pass ? std::numeric_limits<MTR::addr_t>::max() : 0);
      }

      return single;
    }

  public:
    // This constructor is used when it is expected to accumulate several ranges 
    AddressRangeFilter(){}

    // This constructor is a convenience for the case of needing to
    // represent only a single range.
    AddressRangeFilter(MTR::addr_t base, MTR::addr_t limit){
      this->addRange(base, limit);
    }

    // void print() const {
    //   std::cout << "AddressRangeFilter<" << (Pass ? "true" : "false") << ">" << std::endl;
    //   int j = 1;
    //   for(range_vector::const_iterator i = ranges.begin(); i != ranges.end(); i++){
    //     std::cout << "range " << j++ << ": 0x" << std::hex << i->first << " -> 0x" << i->second << std::dec << std::endl;
    //   }
    // }

    void addRange(MTR::addr_t base, MTR::addr_t limit){
      ranges.push_back(std::make_pair(base, limit));
    }

    bool test(const MTR::Record& data){
      return this->consume_helper(data);
    }

    MTR::addr_t firstBase() const {
      return ranges[0].first;
    }

    MTR::addr_t firstLimit() const {
      return ranges[0].second;
    }

    // NOTE(choudhury: C++ is dumb, so the best way to do this is to
    // template the helper method, and repeat code for the consumer
    // method like this.  I can't just template the consume method
    // here because "templates cannot be virtual", according to g++.
    void consume(const MTR::Record& data){
      this->consume_helper(data);
    }

    void consume(const CacheAccessRecord& data){
      this->consume_helper(data);
    }

    void consume(const CacheStatusReport& data){
      this->consume_helper(data);
    }

    template<typename T>
    bool consume_helper(const T& t){
      // NOTE(choudhury): this function is structured in this way for
      // clarity.  The compiler will be able to optimize the
      // if-statement by tossing out one of the code paths (since Pass
      // is a template value parameter and is therefore known at
      // compile time).
      //
      // The true-path works by passing along the reference trace and
      // returning as soon as it is found to lie within any of the
      // ranges.  The false-path does the logical opposite: it returns
      // WITHOUT passing along the record if it is found to lie in any
      // of the ranges; if the function has not returned after testing
      // all of the ranges, the record must lie OUTSIDE all of them,
      // so it is passed along in that case.
      if(Pass){
        for(range_vector::const_iterator i = ranges.begin(); i != ranges.end(); i++) {
          if(i->first <= t.addr and t.addr < i->second){
            this->Filter<T>::produce(t);
            return true;
          }
        }
        return false;
      }
      else{
        for(range_vector::const_iterator i = ranges.begin(); i != ranges.end(); i++) {
          if(i->first <= t.addr and t.addr < i->second){
            return false;
          }
        }

        this->Filter<T>::produce(t);
        return true;
      }
    }

  private:
    range_vector ranges;
  };

  // Convenience names for the two polarities of filter.
  typedef AddressRangeFilter<true> AddressRangePass;
  typedef AddressRangeFilter<false> AddressRangeStop;
}

#endif
