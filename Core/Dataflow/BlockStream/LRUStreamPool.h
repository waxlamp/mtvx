// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// LRUStreamPool.h - A templated class that manages a group of
// streams, allowing access to the underlying streams by some kind of
// ID marker.

#ifndef LRU_STREAM_POOL_H
#define LRU_STREAM_POOL_H

// MTV headers.
#include <Core/Util/Boost.h>

// System headers.
#include <fstream>
#include <list>
#include <stdexcept>
#include <stdint.h>

namespace MTV{
  template<typename Stream, typename Position>
  struct StreamTraits{
    class Unimplemented : std::exception{
    public:
      const char *what() throw(){
        return "Function unimplemented.";
      }
    };

    static Position tell(Stream *){
      char c[0];
      throw Unimplemented();
    }

    static void seek(Stream *, Position){
      throw Unimplemented();
    }

    static Stream *create(const std::string& filename){
      throw Unimplemented();
    }

    static void flush(Stream *){
      throw Unimplemented();
    }
  };

  template<>
  struct StreamTraits<std::ofstream, std::streampos>{
    typedef std::ofstream Stream;
    typedef std::streampos Position;

    static Position tell(Stream *t){
      return t->tellp();
    }

    static void seek(Stream *t, Position pos){
      t->seekp(pos);
    }

    static Stream *create(const std::string& filename){
      // NOTE(choudhury): this is incredibly dumb: in order for the
      // output stream not to assume that locations in the file that
      // it hasn't written to itself aren't simply zero, you have to
      // use append mode, which we cannot use because it works by
      // setting the file pointer to EOF before every single output
      // operation.  The only way around this is to create the
      // ofstream in input/output mode.  I guess the input mode makes
      // it so that the stream won't create zeros to places it hasn't
      // written to upon closing the stream.  Really.  Really.  Dumb.
      return new Stream(filename.c_str(), std::ios_base::in | std::ios_base::out);
    }

    static void flush(Stream *t){
      t->flush();
    }
  };

  template<>
  struct StreamTraits<std::ifstream, std::streampos>{
    typedef std::ifstream Stream;
    typedef std::streampos Position;

    static std::streampos tell(Stream *t){
      return t->tellg();
    }

    static void seek(Stream *t, Position pos){
      t->seekg(pos);
    }

    static Stream *create(const std::string& filename){
      return new Stream(filename.c_str());
    }

    static void flush(Stream *t){
      // This function is intentionally empty.
    }
  };

  template<typename Stream, typename Index, typename Position>
  class LRUStreamPool{
  public:
    class DuplicateEntry : public std::exception{
    public:
      const char *what() throw(){
        return "LRUStreamPool: attempt to add a new block with an exisiting block id.";
      }
    };

  public:
    // Initialize the stream pool with a fixed number of active
    // elements.
    LRUStreamPool(const std::string& filename, unsigned size)
      : filename(filename),
        size(size)
    {}

    ~LRUStreamPool(){
      // Delete the stream objects.
      for(typename std::list<IdStream>::iterator i = active.begin(); i != active.end(); i++){
        StreamTraits<Stream, Position>::flush(i->stream);
        delete i->stream;
      }
    }

    bool addBlockStream(Index id, Position init){
      // Check for duplicate entries.
      if(blockstreams.find(id) != blockstreams.end()){
        throw DuplicateEntry();
      }

      // Create the entry - give it the right initial position, but
      // don't assign a file stream to it.
      blockstreams[id].pos = init;
      blockstreams[id].element = active.end();

      // If the list is not yet full, create an entry (at the front).
      if(active.size() < size){
        // Create the entry itself.
        active.push_front(IdStream());
        active.front().id = id;
        active.front().stream = StreamTraits<Stream, Position>::create(filename);

        if(!(*active.front().stream)){
          return false;
        }

        // Seek the new stream to the right place.
        StreamTraits<Stream, Position>::seek(active.front().stream, init);

        // Log the correct list iterator in the hash table.
        blockstreams[id].element = active.begin();
      }

      return true;
    }

    unsigned numBlockStreams() const {
      return blockstreams.size();
    }

    unsigned numFileStreams() const {
      return active.size();
    }

    // This function gives the user access to the underlying
    // filestream, while also ticking up the last-used value, and
    // managing the order of the list of active stream elements.
    Stream *access(Index id){
      // TODO(choudhury): check the back() of the list and compare to
      // id - short circuit the lookup if the requested stream is also
      // the last accessed stream.

      // Grab the entry from the full table of streams.
      typename boost::unordered_map<Index, StreamState>::iterator i = blockstreams.find(id);
      assert(i != blockstreams.end());

      // typename std::list<StreamState>::iterator stream = i->second;
      const StreamState& state = i->second;

      // Check to see if the id is already active.
      if(state.element != active.end()){
        // Move the element into the last position in the list.
        move_to_end(active, state.element);

        // return state.element->stream;
        return active.back().stream;
      }

      // If id is not already active, we need to toss an entry from
      // the table - the head of the list contains the least recently
      // used item.
      //
      // First, look at the front element, and save the stream
      // position to the table of all block ids.
      const std::streampos cur = StreamTraits<Stream, Position>::tell(active.front().stream);
      typename boost::unordered_map<Index, StreamState>::iterator bs = blockstreams.find(active.front().id);
      assert(bs != blockstreams.end());
      bs->second.pos = cur;

      // Invalidate the pointer in the global table (since it is no
      // longer active).
      bs->second.element = active.end();
      
      // Grab the entry on the block stream.
      // typename boost::unordered_map<Index, typename std::list<StreamState>::iterator>::iterator new_stream = blockstreams.find(id);
      typename boost::unordered_map<Index, StreamState>::iterator new_stream = blockstreams.find(id);
      assert(new_stream != blockstreams.end());

      // Set its list iterator to be the front element of the list (it
      // will move to the back of the list at the end of this
      // operation).
      new_stream->second.element = active.begin();

      // Load new data into the LRU frame.
      //
      // First flush the buffer and reset the stream position.
      StreamTraits<Stream, Position>::flush(active.front().stream);
      StreamTraits<Stream, Position>::seek(active.front().stream, new_stream->second.pos);

      // Now reset the stream id block.
      active.front().id = id;

      // Finally, move the block from the front of the list to the
      // back (as it is now the most-recently used item).
      move_to_end(active, active.begin());

      return active.back().stream;
    }

  private:
    template<typename T>
    inline static void move_to_end(std::list<T>& s, typename std::list<T>::iterator i){
      // Used to hold the moving iterator temporarily.
      static std::list<T> tmp;

      // If it's already at the end of the list, we're done.
      if(i == s.end()){
        return;
      }

      // Splice the iterator to the tmp list.
      tmp.splice(tmp.end(), s, i);

      // Splice it back into the target list.
      s.splice(s.end(), tmp, i);
    }

  private:
    // This is used to (1) know which stream was the least-recently
    // used (via the "last" field) and (2) to reconstruct the stream
    // state for that stream when it moves to a new position.
    struct IdStream{
      IdStream()
        : id(0),
          stream()
      {}

      Index id;
      Stream *stream;
    };

    struct StreamState{
      Position pos;
      typename std::list<IdStream>::iterator element;
    };

    // This list of stream state objects is kept incrementally sorted
    // - when a victim must be chosen, it is always the one at the
    // front of the list; it is converted into a new entry at the back
    // of the list, with concordant updates to the hash table of
    // active elements.
    std::list<IdStream> active;

    // Maps from id to stream objects; when the requested id is not
    // present, it will trigger an LRU replacement event to bring that
    // element into the active set (evicting some other element).
    boost::unordered_map<Index, StreamState> blockstreams;

    const std::string filename;
    unsigned size;
  };

  typedef LRUStreamPool<std::ofstream, uint64_t, std::streampos> LRUOfstreamPool;
  typedef LRUStreamPool<std::ifstream, uint64_t, std::streampos> LRUIfstreamPool;
}

#endif
