// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ItemSelector.h - A filter object that keeps track of several
// widgets, consumes an index, and produces the corresponding widget.

#ifndef ITEM_SELECTOR_H
#define ITEM_SELECTOR_H

namespace MTV{
  template<typename Item>
  class ItemSelector : public Filter<unsigned, Item> {
  public:
    BoostPointers1(ItemSelector, Item);

  public:
    void addItem(Item w){
      items.push_back(w);
    }

    void consume(const unsigned& i){
      this->produce(items[i]);
    }

  private:
    std::vector<Item> items;
  };
}

#endif
