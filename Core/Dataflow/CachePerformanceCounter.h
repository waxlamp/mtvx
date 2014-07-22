// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CachePerformanceCounter.h - Accepts collections of hit records and
// computes the average performance of the various levels - could be
// used either to report these numbers directly or as a guide to
// visualizing "cache temperature" etc.

#ifndef CACHE_PERFORMANCE_COUNTER_H
#define CACHE_PERFORMANCE_COUNTER_H

// MTV headers.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/Filter.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/LevelComparator.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/NewCacheSimulator/NewCache.h>
using Daly::CacheEvictionRecord;
using Daly::CacheHitRecord;
using MTV::NewCache;

// System headers.
#include <numeric>
#include <ostream>

namespace MTV{
  class HitHistory{
  public:
    BoostPointers(HitHistory);

  public:
    HitHistory(unsigned size)
      : id(static_id++),
        infinite(size == static_cast<unsigned>(-1)),
        history(infinite ? 0 : size, 0.0),
        p(0),
        frac(1.0 / size),
        avg(0.0),
        L1(false)
    {}

    void setL1(){
      L1 = true;
    }

    bool isL1() const {
      return L1;
    }

    void record(float val){
      if(!infinite){
        history[p] = val;
        p = (p + 1) % history.size();
      }
      else{
        // Expand the average.
        avg *= p;

        // Add in the new value.
        avg += val;

        // Divide by the new number of items.
        avg /= ++p;
      }
    }

    float hitRate() const {
      // for(unsigned i=0; i<history.size(); i++){
      //   std::cout << history[i] << " ";
      // }
      // std::cout << std::endl;

      // std::cout << "p = " << p << std::endl;

      if(!infinite){
        // TODO(choudhury): interpolate from -4 to 1 for the L2.
        float avg = std::accumulate(history.begin(), history.end(), 0) * frac;

        // std::cout << "avg (before) = " << avg << std::endl;

        if(L1){
          // Map this value, which lies in [-4, 1], to [-1, 1].
          if(avg < 0.0){
            avg /= 4.0;
          }
        }

        // float avg = 0.0;
        // for(unsigned i=0; i<history.size(); i++){
        //   avg += i*history[i];
        // }
        // avg /= (history.size() - 1)*(history.size() - 1) / 2.0;

        // std::cout << "avg = " << avg << std::endl;

        // Average the values in the history vector.
        return avg;
      }
      else{
        return (L1 and avg < 0.0) ? avg / 4.0 : avg;
      }
    }

  private:
    static unsigned static_id;
    unsigned id;

    bool infinite;

    std::vector<float> history;
    int p;

    const float frac;

    float avg;

    bool L1;
  };

  template<typename CacheLevelType>
  class HitHistoryManagerT{
  public:
    BoostPointers(HitHistoryManagerT);

  public:
    HitHistoryManagerT(unsigned size)
      : size(size)
    {}

    HitHistory::ptr getHistory(typename CacheLevelType::const_ptr p){
      if(histories.find(p) == histories.end()){
        histories[p] = boost::make_shared<HitHistory>(size);
      }

      return histories[p];
    }

  private:
    unsigned size;
    boost::unordered_map<typename CacheLevelType::const_ptr, HitHistory::ptr> histories;
  };

  typedef HitHistoryManagerT<Daly::CacheLevel> HitHistoryManager;
  typedef HitHistoryManagerT<CacheLevel> NewHitHistoryManager;

  class CacheHitRates{
  public:
    void add(float rate){
      rates.push_back(rate);
    }

    const std::vector<float>& getRates() const {
      return rates;
    }

    float& operator[](unsigned i){
      return rates[i];
    }

    const float& operator[](unsigned i) const {
      return rates[i];
    }

    friend std::ostream& operator<<(std::ostream& out, const CacheHitRates& c){
      out << "(";
      for(unsigned i=0; i<c.rates.size() - 1; i++){
        out << c.rates[i] << ", ";
      }
      out << *(c.rates.end() - 1) << ")";

      return out;
    }

  private:
    std::vector<float> rates;
  };

  class PerformanceCounterPolicy{
  public:
    BoostPointers(PerformanceCounterPolicy);

  public:
    virtual void consume(const CacheAccessRecord& rec) = 0;
    virtual CacheHitRates rates() const = 0;
  };

  template<typename CacheType>
  class CacheSetUtilizationPolicyT : public PerformanceCounterPolicy {
  public:
    BoostPointers(CacheSetUtilizationPolicyT);

  public:
    CacheSetUtilizationPolicyT(typename CacheType::const_ptr c, unsigned window)
      : history(c->num_levels()),
        blocksize(c->block_size())
    {
      for(unsigned i=0; i<c->num_levels(); i++){
        for(unsigned j=0; j<c->level(i)->associativity(); j++){
          history[i].push_back(boost::make_shared<HitHistory>(window));
        }
      }
    }
    
    void consume(const CacheAccessRecord& rec){
      // Scan through the hit records.
      const unsigned numlevels = history.size();
      for(std::vector<CacheHitRecord>::const_iterator hit=rec.hits.begin(); hit != rec.hits.end(); hit++){
        // If the record hit to memory (a cache miss), skip it.
        if(hit->L == numlevels){
          continue;
        }

        // Otherwise, log a 1 in the appropriate set of the level, and a 0
        // in the other sets.  Leave the other levels alone, as they
        // weren't really involved with this computation.
        const unsigned hit_set = (hit->addr / blocksize) % history[hit->L].size();
        // std::cout << "addr: " << (hit->addr / blocksize) << ", L" << (hit->L+1) << ", set: " << hit_set << std::endl;

        for(unsigned set=0; set<history[hit->L].size(); set++){
          history[hit->L][set]->record(set == hit_set);
        }
      }
      // std::cout << "-----" << std::endl;
    }

    CacheHitRates rates() const {
      CacheHitRates retval;

      // Collect the utilization rates.
      for(unsigned L=0; L<history.size(); L++){
        for(unsigned set=0; set<history[L].size(); set++){
          retval.add(history[L][set]->hitRate());
        }
      }

      return retval;
    }

  private:
    std::vector<std::vector<HitHistory::ptr> > history;
    unsigned blocksize;
  };

  typedef CacheSetUtilizationPolicyT<Daly::Cache> CacheSetUtilizationPolicy;
  typedef CacheSetUtilizationPolicyT<NewCache> NewCacheSetUtilizationPolicy;

  template<typename CacheType, typename HitHistoryManagerType>
  class CacheTemperaturePolicyT : public PerformanceCounterPolicy {
  public:
    BoostPointers(CacheTemperaturePolicyT);

  public:
    CacheTemperaturePolicyT(typename CacheType::const_ptr c, unsigned window)
      : history(c->num_levels())
    {
      for(unsigned i=0; i<history.size(); i++){
        history[i] = boost::make_shared<HitHistory>(window);
      }

      history[0]->setL1();
    }

    CacheTemperaturePolicyT(typename CacheType::const_ptr c, typename HitHistoryManagerType::ptr mgr){
      for(unsigned i=0; i<c->num_levels(); i++){
        history.push_back(mgr->getHistory(c->level(i)));
      }

      history[0]->setL1();
    }

    void consume(const CacheAccessRecord& rec){
      // Find the highest level of cache that was involved in this access.
      //
      // const unsigned highest = std::max_element(rec.hits.begin(), rec.hits.end(), LevelComparator<Daly::CacheHitRecord>())->L;
      //
      // NOTE(choudhury): no, we want the LOWEST level that was hit.
      const unsigned highest = std::min_element(rec.hits.begin(), rec.hits.end(), LevelComparator<Daly::CacheHitRecord>())->L;

      // Update the appropriate hit counts.
      //
      // Every cache level below the highest hit did not contain the
      // sought block - a miss at that level.
      for(unsigned i=0; i<highest; i++){
        // std::cout << "miss at " << i << std::endl;

        // history[i].record(-1.0);

        // Record a -4 for L1 on a miss, because it has the most
        // opportunity to make good on the miss (via fast subsequent
        // references to this block)
        //
        // history[i]->record(i == 0 ? -4.0 : -1.0);
        history[i]->record(history[i]->isL1() ? -4.0 : -1.0);
      }

      // If the "highest" level does not represent main memory (which is
      // represented by the number AFTER the number of cache levels in the
      // cache), then that level hit - that was where the block was found.
      if(highest < history.size()){
        // std::cout << "hit at " << highest << std::endl;
        history[highest]->record(1.0);
      }

      // The levels above "highest" were not involved at all, so we record
      // a neutral value for them.
      for(unsigned i=highest+1; i<history.size(); i++){
        // std::cout << "nothing at " << i << std::endl;
        history[i]->record(0.0);
      }
    }

    CacheHitRates rates() const {
      CacheHitRates rates;
      for(std::vector<HitHistory::ptr>::const_iterator i = history.begin(); i != history.end(); i++){
        rates.add((*i)->hitRate());
      }

      return rates;
    }

  private:
    std::vector<HitHistory::ptr> history;
  };

  typedef CacheTemperaturePolicyT<Daly::Cache, HitHistoryManager> CacheTemperaturePolicy;
  typedef CacheTemperaturePolicyT<NewCache, NewHitHistoryManager> NewCacheTemperaturePolicy;

  template<typename CacheType>
  class LevelToLevelBandwidthPolicyT : public PerformanceCounterPolicy {
  public:
    BoostPointers(LevelToLevelBandwidthPolicyT);

  public:
    LevelToLevelBandwidthPolicyT(typename CacheType::const_ptr c)
      : blocksize(c->block_size())
    {
      // Initialize the bandwidth counter vector with zeroes - one per
      // PAIR of levels (L1-to-L2, L2-to-memory, etc.).
      for(unsigned i=0; i<c->num_levels(); i++){
        bandwidth.add(0.0);
      }
    }

    void consume(const CacheAccessRecord& rec){
      // For each read hit, increment the bandwidth counters to all levels
      // below it.  For a write hit, increment only the bandwidth counter
      // to the level below the hit level (due to the way write hits are
      // accounted).
      // for(std::vector<CacheHitRecord>::const_iterator hit = rec.hits().begin(); hit != rec.hits.end(); hit++){
      foreach(const CacheHitRecord& hit, rec.hits){
        if(hit.op == 'R'){
          // NOTE(choudhury): a read hit to level L means that the highest
          // bandwidth counter to increment is the one from level (L-1) to
          // level L: this counter is in fact indexed by (L-1), so we just
          // increment the counters from 0 to L non-inclusive, as in a
          // standard for-loop.
          for(unsigned i=0; i<hit.L; i++){
            bandwidth[i] += blocksize;
          }
        }
        else if(hit.op == 'W'){
          // Write hits are recorded one at a time, to account for the
          // fact that writeback and writethrough levels must be handled
          // differently.  For each write hit encountered, increment just
          // the appropriate bandwidth counter.
          if(hit.L > 0){
            bandwidth[hit.L - 1] += blocksize;
          }
        }
        else{
          abort();
        }
      }

      // If there was a writeback eviction, increment the appropriate
      // counter.
      foreach(const CacheEvictionRecord& e, rec.evictions){
        if(e.writeback and e.dirty and e.L > 0){
          bandwidth[e.L - 1] += blocksize;
        }
      }
    }

    CacheHitRates rates() const {
      return bandwidth;
    }

  private:
    const float blocksize;
    CacheHitRates bandwidth;
  };

  typedef LevelToLevelBandwidthPolicyT<Daly::Cache> LevelToLevelBandwidthPolicy;
  typedef LevelToLevelBandwidthPolicyT<NewCache> NewCacheLevelToLevelBandwidthPolicy;

  template<typename CacheType>
  class CacheMissCountPolicyT : public PerformanceCounterPolicy {
  public:
    BoostPointers(CacheMissCountPolicyT);

  public:
    CacheMissCountPolicyT(typename CacheType::const_ptr c)
      : hitlevel(c->num_levels()),
        count(0)
    {}

    void consume(const CacheAccessRecord& rec){
      foreach(const CacheHitRecord& hit, rec.hits){
        if(hit.L == hitlevel){
          count++;
        }
      }

      foreach(const CacheEvictionRecord& e, rec.evictions){
        if(e.writeback and e.dirty and e.L == hitlevel){
          count++;
        }
      }
    }
    CacheHitRates rates() const {
      CacheHitRates rates;
      rates.add(count);

      return rates;
    }

  private:
    unsigned hitlevel;
    unsigned count;
  };

  typedef CacheMissCountPolicyT<Daly::Cache> CacheMissCountPolicy;
  typedef CacheMissCountPolicyT<NewCache> NewCacheMissCountPolicy;

  class CachePerformanceCounter : public Filter<CacheAccessRecord, CacheHitRates> {
  public:
    BoostPointers(CachePerformanceCounter);

  public:
    CachePerformanceCounter(unsigned period)
      : period(period)
    {}

    void setPolicy(PerformanceCounterPolicy::ptr p){
      perfcounter = p;
    }

    void consume(const CacheAccessRecord& rec){
      static unsigned call_count = 0;

      // Send the access record to the performance counting engine.
      this->perfcounter->consume(rec);

      // Increment the call counter.  If we are at the end of the
      // recording period, ask the performance counting engine for the
      // current stats, and reset the counter.
      ++call_count;
      if(call_count == period){
        this->produce(perfcounter->rates());
        call_count = 0;
      }
    }

    CacheHitRates rates() const {
      return this->perfcounter->rates();
    }

  private:
    unsigned period;
    PerformanceCounterPolicy::ptr perfcounter;
  };
}

#endif
