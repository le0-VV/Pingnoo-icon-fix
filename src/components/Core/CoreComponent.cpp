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

#include "CoreComponent.h"
#include "ComponentSystem/IComponentManager.h"
#include "PingResult.h"
#include "ContextManager.h"
#include "CommandManager.h"
#include "EditorManager.h"
#include "Core.h"
#include "IRouteEngine.h"
#include "IPingEngineFactory.h"
#include <QDebug>

void CoreComponent::initialiseEvent() {
    qRegisterMetaType<Nedrysoft::Core::PingResult>("Nedrysoft::Core::PingResult");
    qRegisterMetaType<Nedrysoft::Core::RouteList>("Nedrysoft::Core::RouteList");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<Nedrysoft::Core::IPingEngineFactory *>("Nedrysoft::Core::IPingEngineFactory *");

    Nedrysoft::ComponentSystem::addObject(new Nedrysoft::Core::Core());
    Nedrysoft::ComponentSystem::addObject(new Nedrysoft::Core::ContextManager());
    Nedrysoft::ComponentSystem::addObject(new Nedrysoft::Core::CommandManager());
}

void CoreComponent::initialisationFinishedEvent() {
    auto core = Nedrysoft::ComponentSystem::getObject<Nedrysoft::Core::Core>();

    connect(Nedrysoft::Core::IContextManager::getInstance(), &Nedrysoft::Core::IContextManager::contextChanged,
            [&](int newContext, int oldContext) {
                Q_UNUSED(oldContext)
                Nedrysoft::Core::ICommandManager::getInstance()->setContext(newContext);
            });

    core->open();
}
