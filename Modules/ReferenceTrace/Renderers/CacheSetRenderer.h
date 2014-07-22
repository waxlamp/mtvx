// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheSetRenderer.h - Renders a cache set in a similar way as
// CacheRenderer does.

#ifndef CACHE_SET_RENDERER_H
#define CACHE_SET_RENDERER_H

namespace MTV{
  class CacheSetRenderer : public Consumer<CacheEventRenderCommand>,
                           public UpdateNotifier {
  public:
    BoostPointers(CacheSetRenderer);

  public:
    CacheSetRenderer(CacheSet::const_ptr c, unsigned historySize = 5);

    CacheSetDisplay::ptr getWidget() const {
      return cacheSetWidget;
    }

  private:
    CacheSetDisplay::ptr cacheSetWidget;
  };
}

#endif
