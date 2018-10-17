// (c) A. Hanuschkin, 2017
// compile with -std=c++11  because of to_string usage!
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
using namespace Basler_UsbCameraParams;
using namespace Pylon;

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
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
	// The exit code of the sample application.
	int exitCode = 0;
	bool showvid = true; // show the video while recording
	static const uint32_t c_countOfImagesToGrab = 2000;  // number of frames to record

	string dir = "./frames/";  // place to store the video frames
	string filename;
	string spacer;

	// store the frames in cv::Mat
	cv::Mat frame;
	cv::Mat mats[c_countOfImagesToGrab];

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
	camera.AcquisitionFrameRate.SetValue(500.0); // set frame rate
	camera.MaxNumBuffer.SetValue(c_countOfImagesToGrab);
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
	}
	else { cout << "NOTE: cannot set GrabLoopThreadPriority" << endl; }
	//cout << "set GrabLoopThreadPriority" << endl;
	camera.GrabLoopThreadPriority.SetValue(94);

	cout << "InternalGrabEngineThreadPriority: "
		<< camera.InternalGrabEngineThreadPriority.GetValue() << endl;
	if (IsWritable(camera.InternalGrabEngineThreadPriorityOverride))
	{
		camera.InternalGrabEngineThreadPriorityOverride = true;
		//cout << "set InternalGrabEngineThreadPriorityOverwrite" << endl;
	}
	else {
		cout << "NOTE: cannot set InternalGrabEngineThreadPriorityOverride"
			<< endl;
	}
	//cout << "set InternalGrabEngineThreadPriority" << endl;
	camera.InternalGrabEngineThreadPriority.SetValue(95);

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
			uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();
			cv::Mat mat(480, 640, CV_8UC1, pImageBuffer);
			mats[cnt_frames] = mat.clone();
			if (showvid && cnt_frames % 50 == 0) {
				// convert to Mat
				tstart = time(0);
				cv::imshow("Display window", mats[cnt_frames]); // Show our image inside it.
				dt += time(0) - tstart;
				cv::waitKey(waitKeyValue);
				cnt_frames_shown++;
			}
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
	cout << "<t> conversion to Mat: " << dt / float(cnt_frames_shown) << endl;
	cout << "Close the camera." << endl;

	// save to video
	const string NAME = "test.h264";
	cv::VideoWriter outputVideo;
	cv::Size frame_prop;
	frame_prop.width = 640;
	frame_prop.height = 480;
	double fps = 25.;
	bool color = false;

	outputVideo.open(NAME,
		CV_FOURCC('P', 'I', 'M', '1'),
		//CV_FOURCC('X','2','6','4'),
		fps,
		frame_prop,
		color);

	if (stat(cstr_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
		cout << "Writing the frames to files.. this might take some time..." << endl;
		for (int jj = 0; jj < cnt_frames; ++jj) {
			frame = mats[jj].clone();
			outputVideo.write(frame);
			if (jj < 9) { spacer = "000"; }
			if ((jj > 9) && (jj < 99)) { spacer = "00"; }
			if ((jj > 99) && (jj < 999)) { spacer = "0"; }
			if (jj > 999) { spacer = ""; }
			filename = dir + "test_f_" + spacer + to_string(jj) + ".png";
			imwrite(filename, frame);
		}
	}
	else {
		cout << "NOTE: Frames will not be saved. Directory ./frames/ does not exist."
			<< endl;
	}

	outputVideo.release();

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
