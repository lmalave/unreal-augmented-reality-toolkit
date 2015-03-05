/*****************************
 Copyright 2015 (c) Leonardo Malave. All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other materials
 provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY Leonardo Malave ''AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those of the
 authors and should not be interpreted as representing official policies, either expressed
 or implied, of Leonardo Malave.
 ********************************/

#include "OculusARPOC.h"
#include "Engine.h"
#include "OpenCVVideoSource.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

OpenCVVideoSource::OpenCVVideoSource(uint8 cameraIndex, uint16 videoWidth, uint16 videoHeight)
{
    this->CameraIndex = cameraIndex;
    this->VideoWidth = videoWidth;
    this->VideoHeight = videoHeight;
}

OpenCVVideoSource::~OpenCVVideoSource()
{
}

uint16 OpenCVVideoSource::GetVideoWidth() {
    return VideoWidth;
}

void OpenCVVideoSource::SetVideoWidth(uint16 Width) {
    this->VideoWidth = Width;
}

uint16 OpenCVVideoSource::GetVideoHeight() {
    return VideoHeight;
}

void  OpenCVVideoSource::SetVideoHeight(uint16 Height) {
    this->VideoHeight = Height;
}


float OpenCVVideoSource::GetWidthToDistanceRatio() {
    return WidthToDistanceRatio;
}

float OpenCVVideoSource::GetHeightToDistanceRatio() {
    return HeightToDistanceRatio;
}

void OpenCVVideoSource::Init() {
    cv::VideoCapture VidCap(CameraIndex);
    this->VideoCapture = VidCap;
    if (VideoCapture.isOpened()) {
        VideoCapture.set(CV_CAP_PROP_FRAME_WIDTH, VideoWidth);
        VideoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, VideoHeight);
    }
}

void OpenCVVideoSource::Close() {
	VideoCapture.release();
}

void OpenCVVideoSource::GetFrameImage(uint8* DestinationFrameBuffer) {
    cv::Mat Frame;
    VideoCapture >> Frame; // get a new frame from camera
    uint16 Width = Frame.size().width;
    uint16 Height = Frame.size().height;
    uint8* RawFrameBuffer = (uint8*) Frame.data;
    uint8* DestinationPointer = NULL;
    uint8* SourcePointer = NULL;
	
    uint8 RedChannel;
    uint8 GreenChannel;
    uint8 BlueChannel;

    bool CameraUpsideDown = true;
	if (RawFrameBuffer != NULL) {
        if (CameraUpsideDown) { // draw bottom to top and right to left to flip image
            SourcePointer = RawFrameBuffer;
            for (int32 y = 0; y < VideoHeight; y++)
            {
                DestinationPointer = &DestinationFrameBuffer[(VideoHeight - y) * VideoWidth * sizeof(FColor) - 1];
                for (int32 x = 0; x < VideoWidth; x++)
                {
                    BlueChannel  = *SourcePointer++;
                    GreenChannel = *SourcePointer++;
                    RedChannel  = *SourcePointer++;
                    *DestinationPointer-- = 0xFF;
                    *DestinationPointer-- = RedChannel;
                    *DestinationPointer-- = GreenChannel;
                    *DestinationPointer-- = BlueChannel;
               }
            }
        } else {
            DestinationPointer = DestinationFrameBuffer;
            SourcePointer = RawFrameBuffer;
            for (int32 y = 0; y < VideoHeight; y++)
            {
                for (int32 x = 0; x < VideoWidth; x++)
                {
                    *DestinationPointer++ = *SourcePointer++;
                    *DestinationPointer++ = *SourcePointer++;
                    *DestinationPointer++ = *SourcePointer++;
                    *DestinationPointer++ = 0xFF;
                }
            }
        }
    }
}

