# unreal-augmented-reality-toolkit
This is a collection of classes to enable augmented reality development on Unreal Engine

For now the classes just provide a means to draw a webcam video to the background, but the idea is that over time more classes will be added related to computer vision, detecting markers, etc.

Let's go straight to the video - here is a demo of drawing objects (in this case Coherent UI browser views) on top of a video that is in the background:

https://www.youtube.com/watch?v=I0IZ_G_TWIo	

## How to integrate into Unreal Engine C++ project

* To use the OpenCVVideo source, will need to install the OpenCV SDK and add the "core" and "highgui" libraries to the libpath. For OpenCV you will also need to know the index of your webcam.  If you have only one webcam then it is index 0.  If you are on a laptop with a built-in webcam and you are using a second USB webcam (for example mounted to the Oculus),  then the USB webcam will probably be index 1.

* Create a Material called "VideoMaterial" at the path /Game/Materials/ in the Content Browser, using a TextureSample input for the BaseColor, and then convert the TextureSample to a parameter with the name "VideoTexture"

* Create a Blueprint using VideoDisplaySurface as the base class 

add to your Character class constructor: 

	static ConstructorHelpers::FObjectFinder<UBlueprint> 		VideoSurfaceBlueprint(TEXT("Blueprint'/Game/Blueprints/VideoDisplaySurfaceBlueprint.VideoDisplaySurfaceBlueprint'"));
	if (VideoSurfaceBlueprint.Object){
		VideoSurfaceBlueprintClass = (UClass*)VideoSurfaceBlueprint.Object->GeneratedClass;
	}

	BackgroundVideoSurface = ObjectInitializer.CreateDefaultSubobject<UChildActorComponent>(this, TEXT("BackgroundVideoSurface"));
	BackgroundVideoSurface->AttachParent = FirstPersonCameraComponent;
	BackgroundVideoSurface->ChildActorClass = VideoSurfaceBlueprintClass;

add to BeginPlay(): 

	    AVideoDisplaySurface* BackgroundVideoDisplaySurface = (AVideoDisplaySurface*)BackgroundVideoSurface->ChildActor;
	    OpenCVVideoSource* VideoSource = new OpenCVVideoSource(1, 1280, 720);
	    VideoSource->Init();
	    BackgroundVideoDisplaySurface->Init(VideoSource);
		BackgroundVideoSurface->RelativeLocation = FVector(500.f, -0.f, 0.f); // place video surface 5 meters away in virtual space
		BackgroundVideoSurface->RelativeRotation = FRotator(0.f, 90.f, 90.f);
	    BackgroundVideoSurface->RelativeScale3D = FVector(8.89, 5.00, 1.0); // This is for 1280x720 and assuming a 5 meter distance away

That's it!  The VideoDisplaySurface and OpenCV integration is intended to be simple and flexible.   

The actual implementation with OpenCV involves the 3 classes documented below.   

## C++ Class Design and Implementation

The basic idea is to capture video frames from a webcam attached to the HMD, and then draw the video frames to a dynamic texture.  In order to support various video sources, an IVideoSource interface is defined.  For example, OpenCVVideoSource is an implementation for OpenCV, but I have also implemented using the Leap SDK Image API as a source, and also the OvrVision camera as a source (which has its own SDK).   

For now there are 3 classes provided

* IVideoSource - an interface representing a video source that can be displayed on a VideoDisplaySurface
* OpenCVVideoSource - an implementation of the IVideoSource that uses OpenCV to read data from a webcam
* VideoDisplaySurface - is a ShapePlane mesh with a dynamic material that can draw video frames to the dynamic texture

### IVideoSource interface
 
IVideoSource is an interface intended to encapsulate a video source that can be used to provide raw video frame data to be displayed by the VideoDisplaySurface. The implementation class needs implement the GetFrameImage() function, which takes a destination frame image buffer as a parameter.   The VideoSource should read the video frame and write the data to this buffer in RGBA format.   

### OpenCVVideoSource class
 
OpenCVVideoSource is an OpenCV based implementation of the IVideoSource interface.  This class uses OpenCV to read video frames from a standard webcam, and then reformats the data from RGB into RGBA.  The class also supports the camera being mounted upside down (as I am doing with my Logitech C615 mounted updside down on my Oculus)   

### VideoDisplaySurface class
 
The job of the VideoDisplaySurface is to draw the raw RGBA video frame data to a dynamic texture.   The core of the implementation is the UpdateTextureRegions helpfully provided by Epic Games on their wiki: https://wiki.unrealengine.com/Dynamic_Textures and is the reason why the implemenation seems to perform quite well (I'm testing with 1280x720 resolution on a Mac laptop with integrated graphics)

## Roadmap

* Decouple frame update from Tick() so that the game FPS isn't pinned by the webcam FPS.  For augmented reality applications 30 FPS seems to be fine, though.  
* Support for other video sources (I will post Leap Motion and OvrVision implementations soon)
* Additional augmented reality functionality like detecting markers  

