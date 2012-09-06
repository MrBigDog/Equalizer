
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_EVENTCOMMAND_H
#define EQ_EVENTCOMMAND_H

#include <eq/client/api.h>
#include <eq/client/types.h>
#include <co/objectCommand.h>   // base class

namespace eq
{

namespace detail { class EventCommand; }

    /**
     * A command specialization for config events.
     *
     * Event commands are generated by Config::sendEvent and arrive in
     * Config::handleEvent. User data can be extracted with the API provided
     * by co::DataIStream. The user data sent with this command is added while
     * sending the event.
     */
    class EventCommand : public co::ObjectCommand
    {
    public:
        /** @internal */
        EQ_API EventCommand( const co::Command& command );

        /** Copy-construct an event command. @version 1.5.1 */
        EQ_API EventCommand( const EventCommand& rhs );

        /** Destruct an event command. @version 1.5.1 */
        EQ_API ~EventCommand();

        /** @return the event type. @version 1.5.1 */
        EQ_API uint32_t getEventType() const;

    private:
        EventCommand();
        EventCommand& operator = ( const EventCommand& );
        detail::EventCommand* const _impl;

        void _init();
    };

    /** Print the event command to the given output stream. @version 1.5.1 */
    EQ_API std::ostream& operator << ( std::ostream& os,
                                       const EventCommand& command );
}

#endif // EQ_EVENTCOMMAND_H

