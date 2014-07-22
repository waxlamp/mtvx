// Copyright 2010 A.N.M. Imroz Choudhury
//
// LineVisitCounter.cpp

// MTV headers.
#include <Core/Dataflow/LineVisitCounter.h>
using MTV::LineVisitCounter;

void LineVisitCounter::consume(const MTR::Record& rec){
  // Bail if it's not a line record, or if it matches the last line
  // record we saw.
  if(MTR::type(rec) != MTR::Record::LType or (go and rec.file == cur.file and rec.line == cur.line)){
    return;
  }

  // Save this record for comparison to the next one (and acknowledge
  // that we've actually seen a line record).
  cur = rec;
  go = true;

  // Write the current information to the output, and increment the
  // count for the file/line combo.
  if(textfile){
    out << panel->getFrame() << ' ' << rec.file << ' ' << rec.line << ' ' << ++linecount[std::make_pair(rec.file, rec.line)] << std::endl;
  }
  else{
    FD::FrameDump::LineVisitCount::Count *c = visitcount.add_count();
    c->set_file(rec.file);
    c->set_line(rec.line);
    c->set_count(++linecount[std::make_pair(rec.file, rec.line)]);
  }
}
