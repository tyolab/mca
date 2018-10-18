// https://www.optophysiology.uni-freiburg.de/Howto/howto_high_framerate_recordings

// (c) A. Hanuschkin, 2017
// compile with -std=c++11  because of to_string usage!
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;
using namespace Pylon;
using namespace GenApi;

#include <iostream>

#ifdef WITH_CV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

#include <ctime> // tic toc..
#include <string> // use strings
#include <sys/stat.h> // check if dir exist
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
using namespace std;

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

int main(int argc, char* argv[])
{
	// using pylon video recorder
	if (!CVideoWriter::IsSupported())
	{
		std::cout << "VideoWriter is not supported at the moment. Please install the pylon Supplementary Package for MPEG-4 which is available on the Basler website." << endl;
		// Releases all pylon resources. 
		PylonTerminate();
		// Return with error code 1.
		return 1;
	}

	// The exit code of the sample application.
	int exitCode = 0;
	bool showvid = true; // show the video while recording
	static const uint32_t c_countOfImagesToGrab = 2000;  // number of frames to record

	string dir = "./frames/";  // place to store the video frames
	string filename;
	string spacer;

#ifdef WITH_CV
	// store the frames in cv::Mat
	cv::Mat frame;
	cv::Mat mats[c_countOfImagesToGrab];
#else

	CGrabResultPtr results[c_countOfImagesToGrab];
#endif

	// check if frame dir exist
	struct stat sb;
	char * cstr_dir = new char[dir.length() + 1];
	strcpy(cstr_dir, dir.c_str());
	if (stat(cstr_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
		cout << "Frames will be saved in directory ./frames/" << endl;
	}
	else {
		cout << "NOTE: Frames will not be saved. Directory ./frames/ does not exist."
			<< endl;
	}

	// Before using any pylon methods, the pylon runtime must be initialized.
	PylonInitialize();

	CDeviceInfo info;
	info.SetDeviceClass(Camera_t::DeviceClass());
	// Create an instant camera object with the camera device found first.
	Camera_t camera(CTlFactory::GetInstance().CreateFirstDevice(info));

	// Print the model name of the camera.
	cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

	// Open the camera for accessing the parameters.
	camera.Open();

	CIntegerPtr width(camera.GetNodeMap().GetNode("Width"));
	CIntegerPtr height(camera.GetNodeMap().GetNode("Height"));
	CEnumerationPtr pixelFormat(camera.GetNodeMap().GetNode("PixelFormat"));

	// Get camera device information.
	cout << "Camera Device Information" << endl
		<< "=========================" << endl;
	cout << "Vendor           : "
		<< camera.DeviceVendorName.GetValue() << endl;
	cout << "Model            : "
		<< camera.DeviceModelName.GetValue() << endl;
	cout << "Firmware version : "
		<< camera.DeviceFirmwareVersion.GetValue() << endl << endl;

	cout << "Default settings" << endl;
	cout << "================" << endl;
	cout << "Gain          : " << camera.Gain.GetValue() << endl;
	cout << "Exposure time : " << camera.ExposureTime.GetValue() << endl;
	cout << "FPS           : " << camera.AcquisitionFrameRate.GetValue() << endl;
	cout << "MaxNumBuffer  : " << camera.MaxNumBuffer.GetValue() << endl;
	cout << "Width         : " << width->GetValue() << endl;
	cout << "Height        : " << height->GetValue() << endl;

	if (IsWritable(camera.GainAuto))
	{
		camera.GainAuto.FromString("Off");
	}
	double newGain = camera.Gain.GetMin() + ((camera.Gain.GetMax() - camera.Gain.GetMin()) / 4);
	cout << "Gain (25%)    : " << camera.Gain.GetValue()
		<< " (Min: " << camera.Gain.GetMin()
		<< "; Max: " << camera.Gain.GetMax() << ")" << endl;
	camera.Gain.SetValue(0.0);
	camera.ExposureTime.SetValue(1500);
	camera.AcquisitionFrameRate.SetValue(200.0); // set frame rate
	camera.MaxNumBuffer.SetValue(c_countOfImagesToGrab);
	camera.Width.SetValue(800);
	camera.Height.SetValue(640);
	// The number of empty buffers that are not used for grabbing yet.
	//camera.NumEmptyBuffers.SetValue(1000); 
	// The size of the grab result buffer output queue.
	camera.OutputQueueSize.SetValue(c_countOfImagesToGrab);
	// The maximum number of grab results available at any time during a grab session.
	camera.MaxNumGrabResults.SetValue(c_countOfImagesToGrab);

	cout << "GrabLoopThreadPriority: " << camera.GrabLoopThreadPriority.GetValue()
		<< endl;
	if (IsWritable(camera.GrabLoopThreadPriorityOverride))
	{
		camera.GrabLoopThreadPriorityOverride = true;
		//cout << "set GrabLoopThreadPriorityOverwrite" << endl;
			//cout << "set GrabLoopThreadPriority" << endl;
	// camera.GrabLoopThreadPriority.SetValue(34);
	}
	else { cout << "NOTE: cannot set GrabLoopThreadPriority" << endl; }

	cout << "InternalGrabEngineThreadPriority: "
		<< camera.InternalGrabEngineThreadPriority.GetValue() << endl;
	if (IsWritable(camera.InternalGrabEngineThreadPriorityOverride))
	{
		camera.InternalGrabEngineThreadPriorityOverride = true;
		//cout << "set InternalGrabEngineThreadPriorityOverwrite" << endl;
			//cout << "set InternalGrabEngineThreadPriority" << endl;
	//camera.InternalGrabEngineThreadPriority.SetValue(35);
	}
	else {
		cout << "NOTE: cannot set InternalGrabEngineThreadPriorityOverride"
			<< endl;
	}

	cout << endl << "New settings" << endl;
	cout << "=================" << endl;
	cout << "Gain          :  " << camera.Gain.GetValue() << endl;
	cout << "Exposure time :  " << camera.ExposureTime.GetValue() << endl;
	cout << "FPS           :  " << camera.AcquisitionFrameRate.GetValue() << endl;
	cout << "MaxNumBuffer  :  " << camera.MaxNumBuffer.GetValue() << endl;
	cout << "NumEmptyBuffers: " << camera.NumEmptyBuffers.GetValue() << endl;
	cout << "GrabLoopPriority:" << camera.GrabLoopThreadPriority.GetValue() << endl;
	cout << "IntGEPriority :  " << camera.InternalGrabEngineThreadPriority.GetValue()
		<< endl;
	cout << "Width         : " << camera.Width.GetValue() << endl;
	cout << "Height        : " << camera.Height.GetValue() << endl;

	// Number of images to be grabbed.
	time_t tstart = 0, tstartgrab = time(0);
	float dt = 0, dt_rec = 0, dt_total = 0;
	int cnt_frames_shown = 0;
	int cnt_frames = 0;
	uint8_t *pImageBufferArray[c_countOfImagesToGrab];
	int waitKeyValue = 1;

	// Start the grabbing of c_countOfImagesToGrab images.
  // The camera device is parameterized with a default configuration which
  // sets up free-running continuous acquisition.
	cout << endl << "Start Video recording" << endl;
	camera.StartGrabbing(c_countOfImagesToGrab);
	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;
	// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
	// when c_countOfImagesToGrab images have been retrieved.
	while (camera.IsGrabbing())
	{
		//cout<< "try to grab " << endl;
		  // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
		tstart = time(0);
		camera.RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);
		dt_rec += time(0) - tstart;
		// Image grabbed successfully?
		if (ptrGrabResult->GrabSucceeded())
		{
			// Access the image data.
			results[cnt_frames] = ptrGrabResult;
			uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();

#ifdef WITH_CV
			cv::Mat mat(480, 640, CV_8UC1, pImageBuffer);
			mats[cnt_frames] = mat.clone();
			if (showvid && cnt_frames % 50 == 0) {
				// convert to Mat
				tstart = time(0);
				//cv::imshow("Display window", mats[cnt_frames]); // Show our image inside it.
				dt += time(0) - tstart;
				cv::waitKey(waitKeyValue);
				cnt_frames_shown++;
			}
#endif
			cnt_frames++;
		}
		else
		{
			cout << "Error: " << ptrGrabResult->GetErrorCode() << " "
				<< ptrGrabResult->GetErrorDescription() << endl;
		}

	}

	cout << "End Video recording" << endl;
	cout << endl << "=================" << endl;
	cout << "Total time: " << (time(0) - tstartgrab) << "s, Frames Grabbed: "
		<< cnt_frames
		<< " FPS: " << cnt_frames / float(time(0) - tstartgrab) << endl;
	cout << "<t> total per frame:   " << \
		(time(0) - tstartgrab) / float(c_countOfImagesToGrab) << endl;
	cout << "<t> image aquisition:  " << dt_rec / float(cnt_frames) << endl;
	//cout << "<t> conversion to Mat: " << dt / float(cnt_frames_shown) << endl;
	cout << "Close the camera." << endl;

#ifdef WITH_CV
	// save to video
	const string NAME = "test.avi";
	cv::VideoWriter outputVideo;
	cv::Size frame_prop;
	frame_prop.width = 640;
	frame_prop.height = 480;
	double fps = 25.;
	bool color = false;

	//CV_FOURCC('P', 'I', 'M', '1') = MPEG - 1 codec

	//CV_FOURCC('M', 'J', 'P', 'G') = motion - jpeg codec(does not work well)

	//CV_FOURCC('M', 'P', '4', '2') = MPEG - 4.2 codec

	//CV_FOURCC('D', 'I', 'V', '3') = MPEG - 4.3 codec

	//CV_FOURCC('D', 'I', 'V', 'X') = MPEG - 4 codec

	//CV_FOURCC('U', '2', '6', '3') = H263 codec

	//CV_FOURCC('I', '2', '6', '3') = H263I codec

	//CV_FOURCC('F', 'L', 'V', '1') = FLV1 codec
	outputVideo.open(NAME,
		CV_FOURCC('D', 'I', 'V', '3'),
		//CV_FOURCC('P', 'I', 'M', '1'),
		//CV_FOURCC('X','2','6','4'),
		fps,
		frame_prop,
		color);
#else

	// Create a video writer object.
	CVideoWriter videoWriter;

	// The frame rate used for playing the video (playback frame rate).
	const int cFramesPerSecond = 20;
	// The quality used for compressing the video.
	const uint32_t cQuality = 90;

	// Map the pixelType
	CPixelTypeMapper pixelTypeMapper(pixelFormat);
	EPixelType pixelType = pixelTypeMapper.GetPylonPixelTypeFromNodeValue(pixelFormat->GetIntValue());
	// Open the video writer.
			// Set parameters before opening the video writer.
	videoWriter.SetParameter(
		(uint32_t)width->GetValue(),
		(uint32_t)height->GetValue(),
		pixelType,
		cFramesPerSecond,
		cQuality);
	videoWriter.Open("_TestVideo.mp4");
#endif

	// if (stat(cstr_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
		cout << "Writing the frames to files.. this might take some time..." << endl;
		for (int jj = 0; jj < cnt_frames; ++jj) {
			

#ifdef WITH_CV
			frame = mats[jj].clone();
			outputVideo.write(frame);
#else
			videoWriter.Add(results[jj]);
#endif
			if (jj < 9) { spacer = "000"; }
			if ((jj > 9) && (jj < 99)) { spacer = "00"; }
			if ((jj > 99) && (jj < 999)) { spacer = "0"; }
			if (jj > 999) { spacer = ""; }
			filename = dir + "test_f_" + spacer + to_string(jj) + ".jpeg";
			//imwrite(filename, frame);
		}
	//}
	//else {
	//	cout << "NOTE: Frames will not be saved. Directory ./frames/ does not exist."
	//		<< endl;
	//}

#ifdef WITH_CV
		outputVideo.release();
#else
		videoWriter.Close();
#endif

	// Releases all pylon resources.
	try {
		// PylonTerminate();
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}
	return exitCode;
}  // autoInitTerm's destructor calls PylonTerminate() now
