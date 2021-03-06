// OFFIS Automation Framework
// Copyright (C) 2013-2017 OFFIS e.V.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "OutputPortPrivate.h"
#include "PortData.h"

#include <core/PortListener.h>
#include <core/Tracer.h>
#include <filter/Port.h>
#include <gui/OverlayInterface.h>

#include <QDebug>
#include <typeinfo>

OutputPort::OutputPort(Port &parent) : mParent(parent)
{
    parent.portData->out = this;
    mFired = false;
}

OutputPort::OutputPort(Port &parent, bool isList) : mParent(parent)
{
    parent.portData->out = this;
    parent.portData->info.isArray = isList;
    mFired = false;
}


QString OutputPort::name() const
{
    return mParent.name();
}

void OutputPort::send(const QVariant &val)
{
    mLastValueMutex.lock();
    mLastValue = val;
    if(mTracer)
    mTracer->trace(Tracer::PortFired, mTraceId);
    mLastValueMutex.unlock();
    QMutexLocker lock(&mMutex);
    QSetIterator<PortListener*> targets(mTargets);
    while(targets.hasNext())
    {
        PortListener* target = targets.next();
        target->setValue(val);
    }
    mFired = true;
}

PortInfo OutputPort::getInfo()
{
    return mParent.getInfo();
}

void OutputPort::addTarget(PortListener *inputPort)
{
    QMutexLocker lock(&mMutex);
    mTargets.insert(inputPort);
}

void OutputPort::removeTarget(PortListener *inputPort)
{
    QMutexLocker lock(&mMutex);
    mTargets.remove(inputPort);
}

bool OutputPort::hasTargets()
{
    QMutexLocker lock(&mMutex);
    return mTargets.size() > 1;
}

bool OutputPort::isOverlayed()
{
    QMutexLocker lock(&mMutex);
    QSetIterator<PortListener*> targets(mTargets);
    while(targets.hasNext()){
        OverlayInterface *interface = dynamic_cast<OverlayInterface *>(targets.next());
        if(interface){
            return true;
        }
    }
    return false;
}

QVariant OutputPort::lastValue()
{
    mLastValueMutex.lock();
    QVariant var = mLastValue;
    mLastValueMutex.unlock();
    return var;
}

void OutputPort::setTracer(SubTracer *tracer)
{
    mLastValueMutex.lock();
    mTraceId = Tracer::instance().labelId(mParent.name());
    mTracer = tracer;
    mLastValueMutex.unlock();
}

bool OutputPort::isNormalType() const
{
    return mParent.getInfo().type != PartialPortMode;
}

OutputListPort::OutputListPort(Port &parent) : OutputPort(parent, true)
{
}

void OutputListPort::send(const QVariant &var)
{
    mValueList.append(var);
}

void OutputListPort::finish()
{
    OutputPort::send(mValueList);
    mValueList.clear();
}
