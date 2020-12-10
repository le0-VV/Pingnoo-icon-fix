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

#ifndef NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTARGET_H
#define NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTARGET_H

#include "Core/IPingTarget.h"

#if defined(Q_OS_WIN)
#include <WS2tcpip.h>
#include <WinSock2.h>
#endif

namespace Nedrysoft::ICMPSocket {
    class ICMPSocket;
}

namespace Nedrysoft::ICMPPingEngine {
    class ICMPPingTargetData;

    class ICMPPingEngine;

    /**
     * IPingTarget implementation for ICMP
     *
     * Implements the IPingTarget interface to implement a ping target
     * that uses ICMP echo packets for measurements.
     */
    class ICMPPingTarget :
            public Nedrysoft::Core::IPingTarget {

        private:
            Q_OBJECT

            Q_INTERFACES(Nedrysoft::Core::IPingTarget)

        public:
            /**
             * @brief       Constructor
             *
             * @details     Creates a new ping target for a given host and initial TTL
             *
             * @param[in]   engine
             * @param[in]   hostAddress
             * @param[in]   ttl
             */
            ICMPPingTarget(Nedrysoft::ICMPPingEngine::ICMPPingEngine *engine, QHostAddress hostAddress, int ttl = 0);

            /**
             * @brief       Destructor
             */
            ~ICMPPingTarget();

            /**
             * @sa          IPingTarget
             */
            void setHostAddress(QHostAddress hostAddress) override;

            QHostAddress hostAddress() override;

            Nedrysoft::Core::IPingEngine *engine() override;

            void *userData() override;

            void setUserData(void *data) override;

            uint16_t ttl() override;

            /**
             * @sa          IConfiguration
             *
             */
            QJsonObject saveConfiguration() override;

            bool loadConfiguration(QJsonObject configuration) override;

        protected:

            /**
             * @brief       Returns socket to be used to send ICMP packets
             *
             * @return      socket
             */
            Nedrysoft::ICMPSocket::ICMPSocket *socket();

            /**
             * @brief       Returns the ICMP id used for this target
             *
             * @return      the id
             */
            uint16_t id();

            friend class ICMPPingTransmitter;

        protected:
            std::shared_ptr<ICMPPingTargetData> d;
    };
}

#endif // NEDRYSOFT_ICMPPINGENGINE_ICMPPINGTARGET_H