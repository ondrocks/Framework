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

#include "WindowsGamepad.h"

#include <QDebug>
#include <QElapsedTimer>
#include <telecontrol/TcConfig.h>

bool WindowsGamepad::initialize()
{
    setObjectName("Gamepad: " + getName());
    try{
        if(FAILED(mDevice->SetDataFormat( &c_dfDIJoystick2 )))
            throw std::runtime_error(tr("Could not set data format to joystick.").toStdString());

        if(FAILED(mDevice->EnumObjects( WindowsGamepad::enumObject, this, DIDFT_ALL )))
            throw std::runtime_error(tr("Could not enumerate objects").toStdString());

        if(FAILED(mDevice->Acquire()))
            throw std::runtime_error(tr("Could not acquire device").toStdString());
        return true;
    } catch(const std::exception& e) {
        qWarning() << tr("Could not initialize gamepad:") << " " <<  e.what();
        return false;
    }
}

void WindowsGamepad::update(QMap<int, double> &joysticks, QMap<int, bool> &buttons)
{
    DIJOYSTATE2& status = mState;
    joysticks[Tc::XAxisLeft] = correctedValue(float(status.lX));
    joysticks[Tc::YAxisLeft] = correctedValue(-float(status.lY));
    buttons[Tc::LeftAxisPush] = status.rgbButtons[10] != 0;
    joysticks[Tc::XAxisRight] = correctedValue(float(status.lZ));
    joysticks[Tc::YAxisRight] = correctedValue(-float(status.lRz));
    buttons[Tc::RightAxisPush] = status.rgbButtons[11] != 0;

    buttons[Tc::Button1] = status.rgbButtons[0] != 0;
    buttons[Tc::Button2] = status.rgbButtons[1] != 0;
    buttons[Tc::Button3] = status.rgbButtons[2] != 0;
    buttons[Tc::Button4] = status.rgbButtons[3] != 0;
    buttons[Tc::Button5] = status.rgbButtons[4] != 0;
    buttons[Tc::Button6] = status.rgbButtons[5] != 0;
    buttons[Tc::Button7] = status.rgbButtons[6] != 0;
    buttons[Tc::Button8] = status.rgbButtons[7] != 0;
    buttons[Tc::Button9] = status.rgbButtons[8] != 0;
    buttons[Tc::Button10] = status.rgbButtons[9] != 0;

    bool up = false, down = false, left = false, right = false;
    switch(status.rgdwPOV[0]) // contour-clockwise from left direction
    {
        case 9000:  right = true; break;
        case 4500:  right =  true; up =  true; break;
        case 0:     up =  true; break;
        case 31500: left = true; up =  true; break;
        case 27000: left = true; break;
        case 22500: left = true; down = true; break;
        case 18000: down = true; break;
        case 13500: right = true; down = true; break;
    }
    buttons[Tc::ButtonUp] = up;
    buttons[Tc::ButtonDown] = down;
    buttons[Tc::ButtonLeft] = left;
    buttons[Tc::ButtonRight] = right;
}

float WindowsGamepad::correctedValue(float v)
{
    v /= (float)getResolution();
    if(v >-0.1 && v < 0.1)
        return 0.0;
    else
        return v;
}

WindowsGamepad::WindowsGamepad(const QString &name) : mName(name)
{
    mDevice = 0;
}

int WindowsGamepad::getResolution() const
{
    return 64;
}

WindowsGamepad::~WindowsGamepad()
{
    stop();
    wait();
    if(mDevice != 0)
    {
        mDevice->Release();
    }

}

void WindowsGamepad::run()
{
    mStop = false;
    QMap<int,double> joysticks;
    QMap<int,bool> buttons, lastButtons;
    while(!mStop)
    {
        QElapsedTimer timer;
        timer.start();
        if(FAILED(mDevice->GetDeviceState(sizeof(DIJOYSTATE2), &mState)))
            continue;
        update(joysticks, buttons);
        QMapIterator<int,bool> iter(buttons);
        while(iter.hasNext())
        {
            iter.next();
            int id = iter.key();
            bool value = iter.value();
            if(lastButtons.contains(id))
            {
                if(lastButtons[id] != value) // value has changed
                emit buttonToggled(id, value);
            }
            else if(value) // first run, emit if pressed
                emit buttonToggled(id, value);
        }
        lastButtons = buttons;
        emit dataUpdate(joysticks);
        //int remaining = mPollingIntervall - timer.elapsed();
        //if(remaining > 0)
//            msleep(remaining);
    }
}

BOOL CALLBACK WindowsGamepad::enumObject(const DIDEVICEOBJECTINSTANCE* inst, VOID* pContext)
{
	WindowsGamepad* gamepad = (WindowsGamepad*) pContext;


    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( inst->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = inst->dwType; // Specify the enumerated axis
                diprg.lMin              = -gamepad->getResolution();
        diprg.lMax              = +gamepad->getResolution();
    
        // Set the range for the axis
        if( FAILED( gamepad->mDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP;
         
    }
    return DIENUM_CONTINUE;
}
