
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2011, Daniel Nachbaur <danielnachbaur@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "tileQueue.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>


namespace eq
{
namespace server
{

TileQueue::TileQueue()
        : co::Object()
        , _compound( 0 )
        , _size( 0, 0 )
        , _activated( true )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
        _queueMaster[i] = 0;
}

TileQueue::TileQueue( const TileQueue& from )
        : co::Object()
        , _compound( 0 )
        , _name( from._name )
        , _size( from._size )
        , _activated( true )
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
        _queueMaster[i] = 0;
}

TileQueue::~TileQueue()
{
    _compound = 0;
}

void TileQueue::addTile( const TileTaskPacket& tile, fabric::Eye eye )
{
    uint32_t index = co::base::getIndexOfLastBit(eye);
    EQASSERT( index < NUM_EYES );
    _queueMaster[index]->_queue.push( tile );
}

void TileQueue::cycleData( const uint32_t frameNumber, const Compound* compound)
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        if( !compound->isInheritActive( (eq::Eye)(1<<i) ))// eye pass not used
        {
            _queueMaster[i] = 0;
            continue;
        }

        // reuse unused frame data
        latencyQueue* queue    = _queues.empty() ? 0 : _queues.back();
        const uint32_t latency = getAutoObsolete();
        const uint32_t dataAge = queue ? queue->_frameNumber : 0;

        if( queue && dataAge < frameNumber-latency && frameNumber > latency )
            // not used anymore
            _queues.pop_back();
        else // still used - allocate new data
        {
            queue = new latencyQueue;

            getLocalNode()->registerObject( &queue->_queue );
            queue->_queue.setAutoObsolete( 1 ); // current + in use by render nodes
        }

        queue->_frameNumber = frameNumber;

        _queues.push_front( queue );
        _queueMaster[i] = queue;
    }
}

void TileQueue::getInstanceData( co::DataOStream& )
{
}

void TileQueue::applyInstanceData( co::DataIStream& )
{
}

void TileQueue::flush()
{
    unsetData();

    while( !_queues.empty( ))
    {
        latencyQueue* queue = _queues.front();
        _queues.pop_front();
        getLocalNode()->deregisterObject( &queue->_queue );
        delete queue;
    }

}

void TileQueue::unsetData()
{
    for( unsigned i = 0; i < NUM_EYES; ++i )
    {
        _queueMaster[i] = 0;
    }
}

std::ostream& operator << ( std::ostream& os, const TileQueue* tileQueue )
{
    if( !tileQueue )
        return os;
    
    os << co::base::disableFlush << "tileQueue" << std::endl;
    os << "{" << std::endl << co::base::indent;
      
    const std::string& name = tileQueue->getName();
    os << "name      \"" << name << "\"" << std::endl;

    const eq::Vector2i& size = tileQueue->getTileSize();
    os << "tile size \"" << size << "\"" << std::endl;

    os << co::base::exdent << "}" << std::endl << co::base::enableFlush;
    return os;
}

const co::base::UUID TileQueue::getQueueMasterID( fabric::Eye eye )
{
    uint32_t index = co::base::getIndexOfLastBit(eye);
    latencyQueue* queue = _queueMaster[ index ];
    if ( queue )
        return queue->_queue.getID();
    else
        return co::base::UUID::ZERO;
}


}
}
