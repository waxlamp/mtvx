// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// TraceWriter.h - A buffer object that accepts trace records, stores
// them up until the internal buffer is full, and then writes them to
// disk.

#ifndef TRACE_WRITER_H
#define TRACE_WRITER_H

// MTV headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Util/Boost.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// System headers.
#include <fstream>
#include <stdexcept>

namespace MTV{
  class TraceWriter : public Consumer<MTR::Record> {
  public:
    BoostPointers(TraceWriter);

  public:
    enum Encoding {
      Raw = 0,
      Gzip = 1,
      AddressRaw = 2,
      AddressGzip = 3
    };

    static const std::string magicphrase;

  public:
    TraceWriter(Encoding e, size_t size = 256*1024)
      : buf(size),
        p(0),
        out(&outbuf),
        encoding(e)
    {
      // If the requested size is zero, throw an exception.
      if(size == 0){
        throw std::domain_error("'size' parameter for class Buffer cannot be zero.");
      }
    }

    ~TraceWriter(){
      // Write any remaining records to disk.
      this->flush();

      // // Close the output stream.
      // file.close();
    }

    bool open(const std::string& filename){
      // Open the output file, and bail if something goes wrong.
      file.open(filename.c_str(), std::ios::binary);
      if(!file){
        return false;
      }

      // outbuf = new filtering_ostreambuf;
      // out = new std::ostream(outbuf);

      // Write out a magic number and the encoding to the head of the
      // file.
      file << magicphrase << std::flush;
      const unsigned code = static_cast<unsigned>(encoding);
      file.write(reinterpret_cast<const char *>(&code), sizeof(code));

      // Place a gzip compressor in the stream if requested.
      if(encoding == Gzip){
        outbuf.push(gzip_compressor());
      }

      // Close out the pipeline with the filestream.
      outbuf.push(file);

      // return static_cast<bool>(out);
      return static_cast<bool>(file);
    }

    void consume(const MTR::Record& rec){
      this->addRecord(rec);
    }

    void addRecord(const MTR::Record& rec){
      // NOTE(choudhury): The pointer p into the buffer buf is less than
      // buf.size() when this function begins.

      // Copy the record into the buffer and then increment the pointer.
      buf[p++] = rec;

      // The pointer p now MAY equal buf.size(): this indicates that the
      // buffer it full.  Flushing the buffer will reset the pointer to
      // zero.
      if(p == buf.size()){
        this->flush();
      }
    }

    void flush(){
      // for(unsigned i=0; i<buf.size(); i++){
      //   std::cout << buf[i] << std::endl;
      // }

      // Write the bytes of the buffer buf to disk, from index 0 up to
      // but not including the index p.
      out.write(reinterpret_cast<const char *>(&buf[0]), p*sizeof(MTR::Record));
      // out.flush();

      // Reset the index p to zero.
      p = 0;
    }

    // void diskFlush(){
    //   // Filesystem flush.
    //   out.flush();
    // }

  private:
    std::vector<MTR::Record> buf;
    unsigned p;

    std::ofstream file;
    filtering_ostreambuf outbuf;
    std::ostream out;

    Encoding encoding;
  };
}

#endif
