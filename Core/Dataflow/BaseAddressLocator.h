// -*- c++-mode -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// BaseAddressLocator.h - Abstraction over how to figure out what
// address is the base of an object.  Trivially, this may be a
// constant address; non-trivially, it may be relative to some
// changing value, such as a frame base pointer within a stack frame.

#ifndef BASE_ADDRESS_LOCATOR_H
#define BASE_ADDRESS_LOCATOR_H

// MTV headers.
#include <Core/Util/BoostPointers.h>

// System headers.
#include <stdint.h>

namespace MTV{
  class BaseAddressLocator{
  public:
    BoostPointers(BaseAddressLocator);

  public:
    virtual ~BaseAddressLocator(){}

    virtual uint64_t value() const = 0;

    BaseAddressLocator::ptr operator+(const BaseAddressLocator& other) const;
    BaseAddressLocator::ptr operator+(uint64_t other) const;

    // OperatorBaseAddressLocator<std::plus<uint64_t> > operator+(const BaseAddressLocator& other) const {
    //   return OperatorBaseAddressLocator<std::plus<uint64_t> >(*this, other);
    // }
  };

  class ConstantBaseAddress : public BaseAddressLocator {
  public:
    BoostPointers(ConstantBaseAddress);

  public:
    ConstantBaseAddress(uint64_t base)
      : base(base)
    {}

    uint64_t value() const {
      return base;
    }

  private:
    uint64_t base;
  };

  class BaseAddressLocatorSum : public BaseAddressLocator {
  public:
    BoostPointers(BaseAddressLocatorSum);

  public:
    BaseAddressLocatorSum(BaseAddressLocator::const_ptr loc1, BaseAddressLocator::const_ptr loc2)
      : loc1(loc1),
        loc2(loc2)
    {}

    BaseAddressLocatorSum(BaseAddressLocator::const_ptr loc1, uint64_t value)
      : loc1(loc1),
        loc2(boost::make_shared<ConstantBaseAddress>(value))
    {}

    uint64_t value() const {
      return loc1->value() + loc2->value();
    }

  private:
    BaseAddressLocator::const_ptr loc1;
    BaseAddressLocator::const_ptr loc2;
  };

  class RegisterOffsetAddress : public BaseAddressLocator {
  public:
    BoostPointers(RegisterOffsetAddress);

  public:
    uint64_t value() const {
      return static_cast<uint64_t>(base + offset);
    }

    void set_base(uint64_t new_base){
      base = new_base;
    }

    void set_offset(int new_offset){
      offset = new_offset;
    }

  private:
    RegisterOffsetAddress(uint64_t base = 0x0, int offset = 0)
      : base(base),
        offset(offset)
    {}

  private:
    uint64_t base;
    int offset;
  };
}

#endif
