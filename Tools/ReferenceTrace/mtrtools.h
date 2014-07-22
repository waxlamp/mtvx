// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// mtrtools.h - Defines a data structure for dealing with reference
// trace records, and associated utilities.

#ifndef MTRTOOLS_H
#define MTRTOOLS_H

// MTV headers.
#include <Core/Util/ErrorMessage.h>

// System headers.
#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>

namespace MTR{
  using MTV::ErrorMessage;

  typedef uint64_t addr_t;
  typedef uint64_t size_t;

  // Reference trace record.
  struct Record{
    // Codes used to describe this record.
    enum Code {
      NoCode = 0x0,
      Read,          //1
      Write,         //2
      LineNumber,    //3
      FramePointer,  //4
      ScopeEntry,    //5
      ScopeExit,     //6
      FunctionEntry, //7
      FunctionExit   //8
    };

    enum Type {
      NoType = 0x0,
      MType,
      LType,
      OtherType
    };
    
    // Payload data.
    //
    // NOTE(choudhury): this field appears first because it is the
    // largest one in the struct.  In order to have consistent
    // datatype sizes across platforms (especially between 64-bit and
    // 32-bit) this is necessary.  This, however, does NOT address
    // endianness concerns.
    union{
      // Used for Read and Write.
      addr_t addr;

      // Used for LineNumber.
      struct{
        uint32_t file;
        uint32_t line;
      };
    };

    // Record type.
    Code code;

    char pad[4];

    friend std::ostream& operator<<(std::ostream& out, const MTR::Record& rec);
  };

  // Returns a type code based on the record's code field.
  Record::Type type(const Record& rec);

  // Classes for reading XML control information within the trace
  // repository.
  //
  // A linearly depicted region of memory.
  struct ArrayRegion{
    ArrayRegion(addr_t base, size_t size, size_t type=1, std::string title="")
      : base(base), size(size), type(type), title(title) {}

    ArrayRegion()
      : base(0), size(0), type(0), title("") {}

    addr_t base;
    size_t size, type;
    std::string title;
  };

  // Two-dimensionally laid out region of memory.
  struct MatrixRegion{
    MatrixRegion(addr_t base, size_t size, size_t type, int rows, int cols, std::string title="")
      : base(base), size(size), type(type), rows(rows), cols(cols), title(title) {}

    MatrixRegion()
      : base(0), size(0), type(0), rows(0), cols(0), title("") {}

    addr_t base;
    size_t size, type;
    int rows, cols;
    std::string title;
  };

  // This classes parses a registration file and stores information
  // about the memory regions encoded there.
  struct RegionRegistration : public ErrorMessage {
    bool read(const char *filename);

    ArrayRegion barriers;
    ArrayRegion messagespace;
    std::vector<ArrayRegion> arrayRegions;
    std::vector<MatrixRegion> matrixRegions;
  };

  // TODO(choudhury): for some reason, when this function definition
  // appears in in the .cpp file, the linker cannot find it when
  // clients link against this library.  If it appears here inline,
  // then everything seems to be ok.  That's unsettling - best to
  // figure out what's going on.
  inline std::ostream& operator<<(std::ostream& out, const MTR::Record& rec){
    std::stringstream ss;
    ss << "MTR::Record(";

    switch(rec.code){
    case MTR::Record::Read:
      ss << "Read";
      break;

    case MTR::Record::Write:
      ss << "Write";
      break;

    case MTR::Record::LineNumber:
      ss << "LineNumber";
      break;

    case MTR::Record::FramePointer:
      ss << "FramePointer";
      break;

    case MTR::Record::ScopeEntry:
      ss << "ScopeEntry";
      break;

    case MTR::Record::ScopeExit:
      ss << "ScopeExit";
      break;

    case MTR::Record::FunctionEntry:
      ss << "FunctionEntry";
      break;

    case MTR::Record::FunctionExit:
      ss << "FunctionExit";
      break;

    case MTR::Record::NoCode:
      ss << "NoCode";
      break;
    }

    switch(type(rec)){
    case MTR::Record::MType:
      ss << ", 0x" << std::hex << rec.addr << ")";
      break;

    case MTR::Record::LType:
      ss << ", " << rec.file << ", " << rec.line << ")";
      break;

    case MTR::Record::NoType:
      ss << ", NoType)";
      break;

    case MTR::Record::OtherType:
      {
        switch(rec.code){
        case MTR::Record::FramePointer:
        case MTR::Record::ScopeEntry:
        case MTR::Record::ScopeExit:
        case MTR::Record::FunctionEntry:
        case MTR::Record::FunctionExit:
          ss << ", 0x" << std::hex << rec.addr << ")";
          break;

        default:
          ss << ", ERROR)" << std::endl;
          break;
        }
      }
      break;
    }

    out << ss.str();
    return out;
  }

}

#endif
