// Copyright 2010 A.N.M. Imroz Choudhury
//
// LineCacheMissCounter.cpp

// MTV headers.
#include <Core/Dataflow/LineCacheMissCounter.h>
using MTV::LineCacheMissCounter;

void LineCacheMissCounter::consume(const MTR::Record& rec){
  // Bail if it's not a line record.
  if(MTR::type(rec) != MTR::Record::LType){
    return;
  }

  // Seeing a new line record means that we need to set up to collect
  // information about that line.  If there was a previous line
  // record, now is the time to record the information about that
  // line.
  if(go){
    CacheMissCount& count = misscount[std::make_pair(curlinerec.file, curlinerec.line)];
    count.misses += misses;
    count.total += total;
    misses = total = 0;

    // Only record if there are actually misses to report.
    if(count.total > 0){
      if(textfile){
        out << panel->getFrame() << ' ' << curlinerec.file << ' ' << curlinerec.line << ' ' << count.misses << ' ' << count.total << std::endl;
      }
      else{
        // Protocol buffer mode.
        FD::FrameDump::LineMissCount::Count *c = linemisscount.add_count();
        c->set_file(curlinerec.file);
        c->set_line(curlinerec.line);
        c->set_misses(count.misses);
        c->set_total(count.total);
      }
    }
  }

  // Record the file number and line we are setting up for.
  //
  // curfile = rec.file;
  // curline = rec.line;
  curlinerec = rec;

  // Make sure we are good to go (to ensure recording of this record
  // when the next line record comes).
  go = true;
}

void LineCacheMissCounter::consume(const CacheAccessRecord& rec){
  // TODO(choudhury): make the highest level be a parameter to the
  // class instance.
  static const unsigned main_memory_level = 2;

  // Scan through the hit list, counting the misses.
  for(unsigned i=0; i<rec.hits.size(); i++){
    if(rec.hits[i].L == main_memory_level){
      misses++;
    }
  }

  // Note the total number of records.
  total += rec.hits.size();

  // Do the same for the "evictions", noting only those that were
  // writebacks to the main memory level.
  for(unsigned i=0; i<rec.evictions.size(); i++){
    if(rec.evictions[i].writeback){
      // Writebacks result in actual write operations, so count this
      // in the total.
      total++;

      if(rec.evictions[i].L + 1 == main_memory_level){
        // Count it as a miss only if the writeback went to main memory
        misses++;
      }
    }
  }
}
