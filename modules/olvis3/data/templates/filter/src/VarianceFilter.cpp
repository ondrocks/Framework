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

#include "VarianceFilter.h"
#include <opencv2/imgproc.hpp>
REGISTER_FILTER(VarianceFilter);

VarianceFilter::VarianceFilter()
{
    setName("VarianceFilter");
    setDesc("Calculates the variance inside the image or region of interest");
    setGroup("user");
   
	mIn.setName("imageIn");
	addInputPort(mIn);
	
	mRoiIn.setMode(OptionalPortMode);
    mRoiIn.setName("roi");
	addInputPort(mRoiIn);
	
    mVarianceOut.setName("variance");
	addOutputPort(mVarianceOut);
	
    mMeanOut.setName("mean grayscale");
    addOutputPort(mMeanOut);
}

void VarianceFilter::execute()
{
    cv::Mat source = mIn;
	
    cv::Mat image = source;
    if(mRoiIn.hasValue())
    {
        cv::Size size = image.size();
        QRectF imageRect(0,0, size.width, size.height);
        QRectF roi = mRoiIn;
        roi = roi.intersected(imageRect);
        if(roi.isValid())
            image = image(port::Rect::rect(roi));
    }
    cv::Scalar mean, stdDev;
    cv::meanStdDev(image,mean, stdDev);
    double variance = stdDev.val[0]*stdDev.val[0];
    mVarianceOut.send(variance);
    mMeanOut.send(mean[0]);
}
