// OFFIS Automation Framework
// Copyright (C) 2013 OFFIS e.V.
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

#include "OlvisLolec.h"

#include <rc/RcExceptions.h>

#include "ProcessorOutputListener.h"
#include "portobserver.h"
#include "SignalWaiter.h"


OlvisLolec::OlvisLolec() :
    QObject()
{
}

OlvisLolec::~OlvisLolec()
{

}

int OlvisLolec::getFilterId(const QString &name)
{
    FilterInfo info = OlvisCoreInterface::instance()->getFilter(name);
    if(!info.isValid())
        throw RcError(tr("Filter not found: %1").arg(name));
    return info.id;
}

int OlvisLolec::getProcessorId(const QString &name)
{
    ProcessorInfo info = OlvisCoreInterface::instance()->getProcessor(name);
    if(!info.isValid())
        throw RcError(tr("Processor not found: %1").arg(name));
    return info.id;
}

PortId OlvisLolec::getPortId(const QString &filterName, const QString &portName)
{
    PortId id(getFilterId(filterName), portName);
    PortInfo info = OlvisCoreInterface::instance()->getPortInfo(id);
    if(!info.isValid())
        throw RcError(tr("Filter %1 has no port %2").arg(filterName, portName));
    return id;
}

QVariant OlvisLolec::formatReturn(const PortId &id, const QVariant &var)
{
    if(var.type() == QVariant::List)
    {
        QList<QVariant> newList;
        foreach(const QVariant& elem, var.toList())
            newList << OlvisCoreInterface::instance()->portValueToSimpleType(id, elem);
        return newList;
    }
    else
        return OlvisCoreInterface::instance()->portValueToSimpleType(id, var);
}

QVariant OlvisLolec::getPortValue(const QString &filterName, const QString &portName)
{
    PortId id = getPortId(filterName, portName);
    QVariant var = OlvisCoreInterface::instance()->getPortValue(id);
    return formatReturn(id, var);
}


QVariant OlvisLolec::getFuturePortValue(const QString &filterName, const QString &portName, double timeoutInSeconds, int skipValues)
{
    PortId id = getPortId(filterName, portName);

    PortObserver listener(skipValues);
    QVariant var;
    unsigned long timeout = (timeoutInSeconds*1000);
    OlvisCoreInterface::instance()->addPortListener(id, &listener);
    bool success = listener.wait(timeout, var);
    OlvisCoreInterface::instance()->removePortListener(id, &listener);
    if(!success)
        throw RcError(tr("Timeout(%3 s) during getFuturePortValue on %1::%2").arg(filterName, portName).arg(timeoutInSeconds));
    return formatReturn(id, var);
}

QVariant OlvisLolec::getProcessorOutput(const QString &name)
{
    ProcessorOutputListener listener(getProcessorId(name));
    return listener.values();
}

QVariant OlvisLolec::getNextProcessorOutput(const QString &processorName, double timeoutInSeconds)
{
    return getFutureProcessorOutput(processorName, timeoutInSeconds, 0);
}

QVariant OlvisLolec::getFutureProcessorOutput(const QString &name, double timeoutInSeconds, int skipValues)
{
    ProcessorOutputListener listener(getProcessorId(name));
    unsigned long timeout = (timeoutInSeconds*1000);
    bool success = listener.wait(timeout, skipValues);
    if(!success)
        throw RcError(tr("Timeout(%2 s) during getFutureProcessorOutputs on %1").arg(name).arg(timeoutInSeconds));
    return listener.values();
}

QVariant OlvisLolec::getNextPortValue(const QString &filterName, const QString &portName, double timeoutInSeconds)
{
    return getFuturePortValue(filterName, portName, timeoutInSeconds, 0);
}

void OlvisLolec::setPortValue(const QString &filterName, const QString &portName, QVariant var)
{
    PortId id = getPortId(filterName, portName);
    OlvisCoreInterface* olvis = OlvisCoreInterface::instance();
    try
    {
        olvis->setPortValueThrow(id, var);
    }
    catch(...)
    {
        if(var.type() != QVariant::String)
            throw;
        var = olvis->portValueFromString(id, var.toString());
        if(!var.isValid())
            throw;
        olvis->setPortValueThrow(id, var);
    }
}

void OlvisLolec::start()
{
    OlvisCoreInterface::instance()->start();

}

void OlvisLolec::stop()
{
    OlvisCoreInterface::instance()->stop();
}

void OlvisLolec::pauseProcessor(const QString &name)
{
    OlvisCoreInterface::instance()->pauseProcessor(getProcessorId(name));
}

void OlvisLolec::resumeProcessor(const QString &name)
{
    OlvisCoreInterface::instance()->resumeProcessor(getProcessorId(name));
}

