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

#ifndef NEDRYSOFT_HOSTIPGEOAPITPROVIDER_HOSTIGEOPAPIPROVIDER_H
#define NEDRYSOFT_HOSTIPGEOAPITPROVIDER_HOSTIGEOPAPIPROVIDER_H

#include "ComponentSystem/IInterface.h"
#include "Core/IGeoIPProvider.h"
#include "HostIPGeoIPProviderSpec.h"

#include <QObject>
#include <QVariantMap>

namespace Nedrysoft::HostIPGeoIPProvider {
    class Cache;

    /**
     * @brief       IGeoIPProvider implementtation for hostip.com
     *
     * @copydoc     Core::Nedrysoft::IGeoIPProvider
     */
    class HostIPGeoIPProvider :
            public Nedrysoft::Core::IGeoIPProvider {
        Q_OBJECT

            Q_INTERFACES(Nedrysoft::Core::IGeoIPProvider)

        public:
            /**
             * @brief       Constructs the geo ip provider.
             */
            HostIPGeoIPProvider();

            /**
             * @brief       Destroys the geo ip provider.
             */
            ~HostIPGeoIPProvider();

            /**
             * @copydoc     Nedrysoft::Core::IGeoIPProvider::lookup(const QString)
             */
            virtual void lookup(const QString host);

            /**
             * @copydoc     Nedrysoft::Core::IGeoIPProvider::lookup(const QString, Nedrysoft::Core::GeoFunction)
             */
            virtual void lookup(const QString host, Nedrysoft::Core::GeoFunction function);

        private:

        private:
            Nedrysoft::HostIPGeoIPProvider::Cache *m_cache;
    };
}

#endif // NEDRYSOFT_HOSTIPGEOAPITPROVIDER_HOSTIGEOPAPIPROVIDER_H