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

#include "ICMPPingEngine.h"

#include "ICMPPingItem.h"
#include "ICMPPingReceiverWorker.h"
#include "ICMPPingTarget.h"
#include "ICMPPingTimeout.h"
#include "ICMPPingTransmitter.h"
#include "ICMPSocket/ICMPSocket.h"
#include "ICMPPacket/ICMPPacket.h"
#include "Utils.h"

#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <chrono>
#include <cstdint>
#include <spdlog/spdlog.h>

using namespace std::chrono_literals;

constexpr auto DefaultReceiveTimeout = 3s;
constexpr auto DefaultTerminateThreadTimeout = 5s;

using seconds_double = std::chrono::duration<double>;

class Nedrysoft::ICMPPingEngine::ICMPPingEngineData {

    public:
        ICMPPingEngineData(Nedrysoft::ICMPPingEngine::ICMPPingEngine *parent) :
                m_pingEngine(parent),
                m_transmitterWorker(nullptr),
                m_timeoutWorker(nullptr),
                m_transmitterThread(nullptr),
                m_timeoutThread(nullptr),
                m_timeout(DefaultReceiveTimeout),
                m_epoch(std::chrono::system_clock::now()) {

        }

        friend class ICMPPingEngine;

    private:

        Nedrysoft::ICMPPingEngine::ICMPPingEngine *m_pingEngine;

        Nedrysoft::ICMPPingEngine::ICMPPingTransmitter *m_transmitterWorker;
        Nedrysoft::ICMPPingEngine::ICMPPingTimeout *m_timeoutWorker;

        QThread *m_transmitterThread;
        QThread *m_timeoutThread;

        QMap<uint32_t, Nedrysoft::ICMPPingEngine::ICMPPingItem *> m_pingRequests;
        QMutex m_requestsMutex;

        QList<Nedrysoft::ICMPPingEngine::ICMPPingTarget *> m_targetList;

        std::chrono::milliseconds m_timeout = {};

        std::chrono::system_clock::time_point m_epoch;

        Nedrysoft::Core::IPVersion m_version;
};

Nedrysoft::ICMPPingEngine::ICMPPingEngine::ICMPPingEngine(Nedrysoft::Core::IPVersion version) :
        d(std::make_shared<Nedrysoft::ICMPPingEngine::ICMPPingEngineData>(this)) {

    d->m_version = version;

    // timeout thread

    d->m_timeoutWorker = new Nedrysoft::ICMPPingEngine::ICMPPingTimeout(this);

    d->m_timeoutThread = new QThread();

    d->m_timeoutWorker->moveToThread(d->m_timeoutThread);

    connect(d->m_timeoutThread, &QThread::started, d->m_timeoutWorker,
            &Nedrysoft::ICMPPingEngine::ICMPPingTimeout::doWork);

    connect(d->m_timeoutWorker, &Nedrysoft::ICMPPingEngine::ICMPPingTimeout::result, this,
            &Nedrysoft::ICMPPingEngine::ICMPPingEngine::result);

    d->m_timeoutThread->start();

    // connect to the receiver thread

    auto receiver = Nedrysoft::ICMPPingEngine::ICMPPingReceiverWorker::getInstance();

    connect(receiver,
            &Nedrysoft::ICMPPingEngine::ICMPPingReceiverWorker::packetReceived,
            this,
            &Nedrysoft::ICMPPingEngine::ICMPPingEngine::onPacketReceived);

    // transmitter thread

    d->m_transmitterWorker = new Nedrysoft::ICMPPingEngine::ICMPPingTransmitter(this);

    d->m_transmitterThread = new QThread();

    d->m_transmitterWorker->moveToThread(d->m_transmitterThread);

    connect(d->m_transmitterThread, &QThread::started, d->m_transmitterWorker,
            &Nedrysoft::ICMPPingEngine::ICMPPingTransmitter::doWork);

    connect(d->m_transmitterWorker, &Nedrysoft::ICMPPingEngine::ICMPPingTransmitter::result, this,
            &Nedrysoft::ICMPPingEngine::ICMPPingEngine::result);

    d->m_transmitterThread->start();
}

Nedrysoft::ICMPPingEngine::ICMPPingEngine::~ICMPPingEngine() {
    auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(DefaultTerminateThreadTimeout);

    if (d->m_transmitterWorker) {
        d->m_transmitterWorker->m_isRunning = false;
    }

    if (d->m_transmitterThread) {
        d->m_transmitterThread->quit();
    }

    if (d->m_timeoutWorker) {
        d->m_timeoutWorker->m_isRunning = false;
    }

    if (d->m_timeoutThread) {
        d->m_timeoutThread->quit();
    }

    if (d->m_transmitterThread) {
        d->m_transmitterThread->wait(waitTime.count());

        if (d->m_transmitterThread->isRunning()) {
            d->m_transmitterThread->terminate();
        }

        delete d->m_transmitterThread;
    }

    if (d->m_timeoutThread) {
        d->m_timeoutThread->wait(waitTime.count());

        if (d->m_timeoutThread->isRunning()) {
            d->m_timeoutThread->terminate();
        }

        delete d->m_timeoutThread;
    }

    delete d->m_transmitterWorker;
    delete d->m_timeoutWorker;

    for (auto target : d->m_targetList) {
        delete target;
    }

    for (auto request : d->m_pingRequests) {
        delete request;
    }
}

Nedrysoft::Core::IPingTarget *Nedrysoft::ICMPPingEngine::ICMPPingEngine::addTarget(QHostAddress hostAddress) {
    auto target = new Nedrysoft::ICMPPingEngine::ICMPPingTarget(this, hostAddress);

    d->m_transmitterWorker->addTarget(target);

    return target;
}

Nedrysoft::Core::IPingTarget *Nedrysoft::ICMPPingEngine::ICMPPingEngine::addTarget(QHostAddress hostAddress, int ttl) {
    auto target = new Nedrysoft::ICMPPingEngine::ICMPPingTarget(this, hostAddress, ttl);

    d->m_transmitterWorker->addTarget(target);

    d->m_targetList.append(target);

    return target;
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::removeTarget(Nedrysoft::Core::IPingTarget *target) {
    Q_UNUSED(target)

    return true;
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::start() {
    return true;
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::stop() {
    return true;
}

void Nedrysoft::ICMPPingEngine::ICMPPingEngine::addRequest(Nedrysoft::ICMPPingEngine::ICMPPingItem *pingItem) {
    QMutexLocker locker(&d->m_requestsMutex);

    auto id = Nedrysoft::Utils::fzMake32(pingItem->id(), pingItem->sequenceId());

    d->m_pingRequests[id] = pingItem;
}


void Nedrysoft::ICMPPingEngine::ICMPPingEngine::removeRequest(Nedrysoft::ICMPPingEngine::ICMPPingItem *pingItem) {
    QMutexLocker locker(&d->m_requestsMutex);

    auto id = Nedrysoft::Utils::fzMake32(pingItem->id(), pingItem->sequenceId());

    if (d->m_pingRequests.contains(id)) {
        d->m_pingRequests.remove(id);

        pingItem->deleteLater();
    }
}

Nedrysoft::ICMPPingEngine::ICMPPingItem *Nedrysoft::ICMPPingEngine::ICMPPingEngine::getRequest(uint32_t id) {
    QMutexLocker locker(&d->m_requestsMutex);

    if (d->m_pingRequests.contains(id)) {
        return d->m_pingRequests[id];
    }

    return nullptr;
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::setInterval(std::chrono::milliseconds interval) {
    return d->m_transmitterWorker->setInterval(interval);
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::setTimeout(std::chrono::milliseconds timeout) {
    d->m_timeout = timeout;

    return true;
}

void Nedrysoft::ICMPPingEngine::ICMPPingEngine::timeoutRequests() {
    QMutexLocker locker(&d->m_requestsMutex);
    QMutableMapIterator<uint32_t, Nedrysoft::ICMPPingEngine::ICMPPingItem *> i(d->m_pingRequests);

    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    while (i.hasNext()) {
        i.next();

        auto pingItem = i.value();

        std::chrono::duration<double> diff = startTime - pingItem->transmitTime();

        if (diff > d->m_timeout) {
            pingItem->lock();
            if (!pingItem->serviced()) {
                pingItem->setServiced(true);
                pingItem->unlock();

                Nedrysoft::Core::PingResult pingResult(
                        pingItem->sampleNumber(),
                        Nedrysoft::Core::PingResult::NoReply,
                        QHostAddress(), pingItem->transmitEpoch(),
                        diff,
                        pingItem->target() );

                emit result(pingResult);

                i.remove();

                delete pingItem;
            } else {
                pingItem->unlock();
            }
        }
    }
}

QJsonObject Nedrysoft::ICMPPingEngine::ICMPPingEngine::saveConfiguration() {
    return QJsonObject();
}

bool Nedrysoft::ICMPPingEngine::ICMPPingEngine::loadConfiguration(QJsonObject configuration) {
    Q_UNUSED(configuration)

    return false;
}

void Nedrysoft::ICMPPingEngine::ICMPPingEngine::setEpoch(std::chrono::system_clock::time_point epoch) {
    d->m_epoch = epoch;
}

std::chrono::system_clock::time_point Nedrysoft::ICMPPingEngine::ICMPPingEngine::epoch() {
    return d->m_epoch;
}

Nedrysoft::Core::IPVersion Nedrysoft::ICMPPingEngine::ICMPPingEngine::version() {
    return d->m_version;
}

void Nedrysoft::ICMPPingEngine::ICMPPingEngine::onPacketReceived(std::chrono::time_point < std::chrono::high_resolution_clock > receiveTime,
                            QByteArray receiveBuffer, QHostAddress receiveAddress) {

    Nedrysoft::Core::PingResult::PingResultCode resultCode = Nedrysoft::Core::PingResult::NoReply;

    auto responsePacket = Nedrysoft::ICMPPacket::ICMPPacket::fromData(
            receiveBuffer,
            static_cast<Nedrysoft::ICMPPacket::IPVersion>(this->version()) );

    if (responsePacket.resultCode() == Nedrysoft::ICMPPacket::Invalid) {
        return;
    }

    if (responsePacket.resultCode() == Nedrysoft::ICMPPacket::EchoReply) {
        resultCode = Nedrysoft::Core::PingResult::Ok;
    }

    if (responsePacket.resultCode() == Nedrysoft::ICMPPacket::TimeExceeded) {
        resultCode = Nedrysoft::Core::PingResult::Ok;
    }

    auto pingItem = this->getRequest(Nedrysoft::Utils::fzMake32(responsePacket.id(), responsePacket.sequence()));

    if (pingItem) {
        pingItem->lock();

        if (!pingItem->serviced()) {
            pingItem->setServiced(true);
            pingItem->unlock();

            std::chrono::duration<double> diff = receiveTime - pingItem->transmitTime();

            auto pingResult = Nedrysoft::Core::PingResult(pingItem->sampleNumber(),
                                                          resultCode,
                                                          receiveAddress,
                                                          pingItem->transmitEpoch(), diff, pingItem->target() );

            emit Nedrysoft::ICMPPingEngine::ICMPPingEngine::result(pingResult);
        } else {
            pingItem->unlock();
        }

        this->removeRequest(pingItem);

        delete pingItem;
    }
}