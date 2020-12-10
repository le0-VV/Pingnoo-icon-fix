/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of pingnoo (https://github.com/fizzyade/pingnoo)
 * An open source ping path analyser
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTIMEOUT_H
#define NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTIMEOUT_H

#include "Core/PingResult.h"

namespace Nedrysoft::ICMPPingEngine {
    class ICMPPingEngine;

    /**
     * @brief       ICMP packet timeout thread implementation
     *
     * @details     Created and used by the ICMP engine, the timeout thread
     *              calls the engine periodically to detect timed out requests
     */
    class ICMPPingTimeout :
            public QObject {

        private:
            Q_OBJECT

        public:

            /**
             * @brief       Constructor with engine
             *
             * @details     Creates the receiver object and passes in the engine
             *              so that the requests can be tagged to the correct engine
             *
             * @param[in]   engine the owner engine
             */
            ICMPPingTimeout(Nedrysoft::ICMPPingEngine::ICMPPingEngine *engine);

            /**
             * @brief       The receiver thread worker
             */
            Q_SLOT void doWork();

            /**
             * @brief       Signals when a timeout result is available
             *
             * @param[in]   result the result
             */
            Q_SIGNAL void result(Nedrysoft::Core::PingResult result);

            friend class ICMPPingEngine;

        private:
            Nedrysoft::ICMPPingEngine::ICMPPingEngine *m_engine;    //! The engine that owns this timeout worker

        protected:
            bool m_isRunning;                               //! Thread is running
    };
}

#endif // NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTIMEOUT_H
