
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_CHANNEL_H
#define EQ_PIXELBENCH_CHANNEL_H

#include <eq/eq.h>

namespace eqPixelBench
{
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent );
    virtual ~Channel() {}

protected:
    virtual void frameStart( const uint32_t frameID,
                             const uint32_t frameNumber );
    virtual void frameDraw( const uint32_t frameID );

private:
    eq::Frame _frame;
};
}

#endif // EQ_PIXELBENCH_CHANNEL_H

