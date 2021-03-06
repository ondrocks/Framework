// OFFIS Automation Framework
// Copyright (C) 2013-2014 OFFIS e.V.
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

#include "LinuxTelecontrolFactory.h"

#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QPluginLoader>
#include <telecontrol/GamepadInterface.h>
#include <stdexcept>

Gamepad* LinuxTelecontrolFactory::sGamepad;

LinuxTelecontrolFactory::LinuxTelecontrolFactory(){}

LinuxTelecontrolFactory::~LinuxTelecontrolFactory(){}

LinuxTelecontrolFactory &LinuxTelecontrolFactory::instance()
{
    static LinuxTelecontrolFactory factory;
    return factory;
}

Gamepad* LinuxTelecontrolFactory::createGamepad()
{
	// force constructor/destructor call to acquire/release sDirectInput
    instance();
	sGamepad = 0;
    try {
        // Create device
        LinuxGamepad* gamepad = new LinuxGamepad();

        // Try to init device
        if(!gamepad->initialize()){
            delete gamepad;
            throw std::runtime_error(qPrintable(tr("Error initializing gamepad on port /dev/input/js0")));
        } else {
            sGamepad = gamepad;
        }
    }  catch(const std::exception& e) {
        qWarning() << tr("Could not create device:") << " " << e.what();
    }

    return sGamepad;
}
