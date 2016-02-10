// OFFIS Automation Framework
// Copyright (C) 2013-2016 OFFIS e.V.
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

#include "Invert.h"

REGISTER_FILTER(Invert);

Invert::Invert()
{
    setName("Invert");
    setDesc(QObject::tr("Inverts the pixel values of each channel of the image"));
    setGroup("image/color");

    mOut.setName("imageOut");
    mOut.setDesc(QObject::tr("Image output"));
    addOutputPort(mOut);

    mIn.setName("imageIn");
    mIn.setDesc(QObject::tr("Image input"));
    addInputPort(mIn);
}

void Invert::execute()
{
    cv::Mat mat = mIn.getValue().clone();
    uchar* data = (uchar*)mat.datastart;
    while(data < mat.dataend)
    {
        *data = 255-*data;
        data++;
    }
    mOut.send(mat);
}
