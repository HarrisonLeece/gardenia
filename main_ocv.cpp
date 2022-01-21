
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "thread_pool.hpp"
#include <thread>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "canvas.cpp"

#include <chrono>

using namespace std;
using namespace cv;
//namespace fs = std::experimental::filesystem::v1;
//namespace fs = std::filesystem;

// ....

//Use this code ot execture a bash script from CPP
//system("my_bash_script.sh");


void conv_vid_png(string input_path, string output_path){

  VideoCapture cap(input_path);
  if(!cap.isOpened()){
	   cout << "Error opening video stream or file" << endl;
	  }
  for (int count = 1; cap.get(CAP_PROP_FRAME_COUNT)+1; count++ ) {
    Mat frame;
    //Why the fuck does this use the stream insertion operator?
    // >> appears to both increment and store
    cap >> frame;
    //Break the loop when there are no more frames to capture
    if (frame.empty())
	      break;
    //troubleshooting segmentation fault
    try {
      if (frame.channels()<4) {
        //cout << frame.channels() << endl;
        imwrite(output_path+"frame"+to_string(count)+".png", frame);
      }
    }
    catch (int pp) {
      int channels = frame.channels();
      cout << "Invlid frame: "  + to_string(count) << endl;
      cout << "Numer of channels: "  + to_string(channels) << endl;
    }

    cout << "frame " + to_string(count) + " converted to png" << endl;
    //count++;
  }
}

int count_files_in_dir(fs::path inspection_path)
{
  auto dirIter = fs::directory_iterator(inspection_path);
  int fileCount = 0;

  for (auto& entry : dirIter)
  {
      if (fs::is_regular_file(entry))
      {
          ++fileCount;
      }
  }
  return fileCount;
}

void unitTests(){
  cout << "initiating unit tests in main.cpp" << endl;
  //initialie vizModules
  vizModule pngTest1;
  pngTest1.setPosition(2, 0,0,9);
  pngTest1.setModSize(1920, 1080);
  pngTest1.setTiming(0,1);
  pngTest1.addPath("./unit_tests_folder/testpng1/","png");
  vizModule pngTest2;
  pngTest2.setPosition(3, 1920,0,7);
  pngTest2.setModSize(1920,1080);
  pngTest2.setTiming(0,1);
  pngTest2.addPath("./unit_tests_folder/testpng2/","png");
  //vizModule webmTest;
  //vizModule tiffTest;
  //vizModule mp4Test;
  //initialize vCanvas
  vCanvas unitTest;
  unitTest.doConversion = false; //change to false if data is already converted
  unitTest.setSize(1920,1080); //size in px
  unitTest.setVideoParameters(10, 60); //10 seconds, 60 fps
  int numThreads = thread::hardware_concurrency();
  unitTest.setThreads(numThreads - 2); // I have 12 threads, so 10 will work for me.  Use 2 less than the number available to you
  //unitTest.setThreads(1);
  //Load vizModules into vCanvas
  unitTest.addVizModule(pngTest1);
  unitTest.addVizModule(pngTest2);

  //start the Render
  unitTest.renderCanvas();


}

void rpEnough(){
  cout << "initiating Enough routine in main.cpp" << endl;
  //initialie vizModules
  //initialie vizModules
  vizModule universe;
  universe.setPosition(2, 0,0,11);
  universe.setModSize(3820, 2160);
  universe.setTiming(0,1);
  universe.addPath("./universe/","png");
  vizModule psds;
  psds.setPosition(2, 0,0,9);
  psds.setModSize(1554, 1554);
  psds.setTiming(0,1);
  psds.addPath("./psd/","png");
  vizModule waveforms;
  waveforms.setPosition(3, 3820,0,7);
  waveforms.setModSize(1776, 888);
  waveforms.setTiming(0,1);
  waveforms.addPath("./waveform/","png");
  //vizModule webmTest;
  //vizModule tiffTest;
  //vizModule mp4Test;
  //initialize vCanvas
  vCanvas enough;
  enough.doConversion = false; //change to false if data is already converted
  enough.setSize(3820, 2160); //size in px
  enough.setVideoParameters(180, 60); //180 seconds, 60 fps
  int numThreads = thread::hardware_concurrency();
  //enough.setThreads(numThreads - 2); // I have 12 threads, so 10 will work for me.  Use 2 less than the number available to you
  enough.setThreads(10);
  //Load vizModules into vCanvas
  enough.addVizModule(universe);
  enough.addVizModule(psds);
  enough.addVizModule(waveforms);


  //start the Render
  //enough.renderCanvas();
  enough.renderCanvas();
}

int main()
{

  bool unitTesting = false;
  if (unitTesting) {
    unitTests();
    return 55;
  }
  rpEnough();



  return 0;
}
