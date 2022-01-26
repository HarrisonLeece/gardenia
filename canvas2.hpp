#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>    // std::sort
#include <thread>
#include "thread_pool.hpp"
#include "vizModule.hpp"
#include <exception>
#include "shiba_image.hpp"



//for creating directories on disk
namespace fs = std::filesystem; //https://stackoverflow.com/questions/50960492/creating-folders-in-c/56262869
using namespace std;

#define IS_TRUE(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

class vCanvas {
  public:
    //coordinate (0,0) is at the bottom left corner of the canvas
    //display settings
    int width = 1920; //default
    int height = 1080; // default
    int centerWidth = width/2; //location of the center of the canvas
    int centerHeight = height/2;

    //vector<int> backgroundRGBA = {255,255,255,255}; //RGBA, defaults to black
    Scalar backgroundRGBA = Scalar(0,0,0,255); //of type cv::Scalar

    //diagnostics
    bool isTesting = true;

    //convert Modules to usable
    bool doConversion = false;

    vector<vizModule> vizModules;
    int threads = 4; //For my 6 core computer, I like to use 10, for most people, 4 would be easier

    //animation settings
    float videoLengthTime=0; // in seconds.  0 means derive length from vizModules
    int frameRate = 60;

    //output settings
    string outPath = "./super_imposed_files/";
    string format = "png";
    string fileName = "compositedOut";

    void setSize(int width, int height);
    void setVideoParameters(float time, int fps);
    void addVizModule(vizModule in);
    void setBackgroundRGBA(int r, int g, int b, int a); //8 bit color



    void setThreads(int numThreads); // Can be set with canvas.threads  the number of threads aren't utilized until render time
    void renderCanvas();
    //void renderCanvas_1T();

  private:
    vector<vizModule> sortByDepth(); //Sort vizModules so that the deepest one (most bottom) is first
    void convertVideoToUsable(vizModule targetModule);
    void conditionFolder(vizModule target); //add arugments to this
    //void superimpose(int frameNumber, synced_stream output);

};

void vCanvas::setSize(int widthIn, int heightIn) {
  width = widthIn;
  height = heightIn;
  centerWidth = widthIn/2;
  centerHeight = heightIn/2;
}

void vCanvas::setVideoParameters(float time, int fps){
  videoLengthTime = time;
  frameRate = fps;
}

void vCanvas::addVizModule(vizModule in) {
  vizModules.push_back(in);
}

void vCanvas::setBackgroundRGBA(int r, int g, int b, int a) {
  assert(r<255 && g<255 && b<255 && a<255);
  //backgroundRGBA[0] = r;
  //backgroundRGBA[1] = g;
  //backgroundRGBA[2] = b;
  //backgroundRGBA[3] = a;
}

void vCanvas::setThreads(int numThreads){
  assert(numThreads < thread::hardware_concurrency()-1); //numThreads must be 2 threads less than total available threads
  threads = numThreads;
}

//sort the vizModules vector by depths so that the deepest module is first in the list
vector<vizModule> vCanvas::sortByDepth()
{
  vector<vizModule> temp = vizModules;
  vector<vizModule> outVec;
  int deepestIndex = -1;
  int deepestValue = -99999999;
  for (int i=0;i<vizModules.size(); i++)
  {
    for(int k = 0; k < temp.size(); k++)
    {
      if(deepestValue < temp[k].depth)
      {
        deepestValue = temp[k].depth;
        deepestIndex = k;
      }
    }
    //after finding the deepest in the array and it's index, add the deepest value in temp to the out list
    outVec.push_back(temp[deepestIndex]);
    temp.erase(temp.begin()+deepestIndex);
  }
  if (isTesting) //Run tests if testing this module, test sorting algo
  {
    assert(vizModules.size() == outVec.size());
    int storedValue = 2147483647; //max int
    for (int i=0; i < outVec.size(); i++)
    {
      assert(storedValue >= outVec[i].depth);
      storedValue = outVec[i].depth;
    }
  }
  return outVec; //This should now contain the sorted vector of vizModules
}

//see https://www.cplusplus.com/reference/algorithm/sort/
bool alphaNumerSort(string i, string j){
  string shorterStr;
  (i.size() <= j.size()) ? shorterStr = i : shorterStr = j;

  for (int k = 0; k < shorterStr.size(); k++)
  {
    if (i[k] != j[k] && isdigit(i[k]) && isdigit(j[k]) ) {
      if (i.size() == j.size()) {
        return i[k] < j[k];
      }
      return (i.size() < j.size());
    }
    if (i[k] != j[k]) {
      return i[k] < j[k];
    }
  }
  return false;
  //returns true if i is alphabetically before j
}

/*
This function conditions folders into a standard format with the following specifications
The first file in the folder to render has an index starting at 0
name of outPutName.  dataType is preserved
*/
void vCanvas::conditionFolder(vizModule target){
  string outputName = "output";
  string oPath = target.path;
  string temp;
  vector<string> fileStrings;
  cout << "vCanvas::conditionFolder is processing " << oPath << endl;
  //https://en.cppreference.com/w/cpp/filesystem/rename
  for (const auto & entry : fs::directory_iterator(oPath)){
    if (entry.is_regular_file()){
      temp = entry.path().stem().string();
      fileStrings.push_back(temp);
    }
  }
  for (int i = 0; i < fileStrings.size(); i++){
    cout << fileStrings[i] << ",";
  }

  sort(fileStrings.begin(), fileStrings.end(),alphaNumerSort);
  cout << "Directory sorted with canvas.cpp alphaNumerSort" << endl;
  for (int i = 0; i < fileStrings.size(); i++){
    cout << fileStrings[i] << ",";
  }
  cout << endl;
  for (int k = 0; k < fileStrings.size(); k++){
    temp = fileStrings[k];
    cout << oPath+temp << " modified to " << oPath+(outputName+to_string(k)+"."+target.dataType) << endl;
    fs::rename(oPath+temp+"."+target.dataType, oPath+(outputName+to_string(k)+"."+target.dataType));
  }
}

//This does not have to be a method of canvas, its used only in the canvas method
//vCanvas::convertVideoToUsable, which handles some logic
void convertVid(vizModule input) {
  string input_path = input.path;
  string output_path = input.path;
  VideoCapture cap(input_path);
  if(!cap.isOpened()){
	   cout << "Error opening video stream or file" << endl;
	  }
  for (int count = 0; cap.get(CAP_PROP_FRAME_COUNT); count++ ) {
    Mat frame;
    //Why the fuck does this use the stream insertion operator?
    // >> appears to both increment and store
    cap >> frame;
    //Break the loop when there are no more frames to capture
    if (frame.empty())
	      break;
    //for troubleshooting segmentation fault
    try {
      if (frame.channels()==4) {
        cout << "Writing 4 channel frame (RGBA)" << endl;
        //cout << frame.channels() << endl;
        imwrite(output_path+"frame"+to_string(count)+".png", frame);
      } else if (frame.channels()==3)
      {
        cout << "Writing 3 3 channel frame" << endl;
        imwrite(output_path+"frame"+to_string(count)+".png", frame);
      } else {
        cout << "Error in canvas.cpp::convertVid; number of channels in stream is neither 4 (RGBA) or 3 (RGB)" << endl;
        assert(false); // simple way to make the program stop and throw an error
      }
    }
    catch (int pp) {
      int channels = frame.channels();
      cout << "Invlid frame: "  + to_string(count) << endl;
      cout << "Numer of channels: "  + to_string(channels) << endl;
    }

    cout << "frame " + to_string(count) + " converted to png" << endl;
    cout << "Named: "  << "placeholder" << endl;
    //count++;
  }
  //The files should now be converted to .png format with approprite alpha channel
  //Alpha channel https://github.com/opencv/opencv/pull/13395
}

void vCanvas::convertVideoToUsable(vizModule targetModule) {

  if (targetModule.dataType == "tiff" || targetModule.dataType == "png") {
    //Only have to condition the folder to a standard format
    conditionFolder(targetModule);
  } else if (targetModule.dataType == "mp4" || targetModule.dataType == "webm") {
    //first, losslessly convert the file to png through openCV
    convertVid(targetModule); //
    //The output files naturally have format frameX.png, starting at 0,
    //which is determined by settings located in the connvertVidPNG fxn in this class
  }
}

//parse the module and background sizes and positioning arguments into
//start and stop indexes to be used in the fuction superimpose
vector<int> computeBounds(vizModule module, int bgWidth, int bgHeight)  //Should not return void, should return start and end for both the
{
  //////
  //the 0,0 is always the bottom left corner of the canvas for the user, but the matrix uses
  //0,0 as the top left corner
  //Therefore, the x and y coordinates must be transformed to make sense with the
  //the matrix indexes, before any more positioning is done
  //The +y also becomes 'down', + x stays 'right'

  //This is simple once understood, just
  int y = (-1*module.ul_y) + bgHeight; //Add the bgHeight to y
  int x = module.ul_x;

  //out always has the following structure
  //0:vStart, 1: vEnd, 2: hStart, 3:hEnd
  vector<int> out(4,0); //initialize with length 4, data = 0 so we can replace by index

  //Index start and termination selection
  if (x<0) //horizontal start index control
    {out[2] = 0;}
  else
    {out[2] = x;}
  if ((x+module.sizex)>bgWidth) //Horizontal end index control
    {out[3] = bgWidth;}
  else
    {out[3] = x+module.sizex;}
  //vertical logic
  if (y<0)
    {out[0] = 0;}
  else
    {out[0] = y;}
  if ((y+module.sizey)>bgHeight)
    {out[1] = bgHeight;}
  else
    {out[1] = y+module.sizey;}

  return out;
}

string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

//This function cannot be a class member function
//Therefore pass a million arguments to it.  Optional last arg , synced_stream output
void superimpose(int frameNumber_in, vector<vizModule> vizModules, Mat bckg, int frameRate)
{
  try {
    int frameNumber = frameNumber_in;
    vector<vizModule> renderStack; //the set of ordered vizModules sent to the render pipeline
    //Control which vizModules enter the render stack for this frame
    for (int i = 0; i < vizModules.size(); i++) {
      if (vizModules[i].isActive(frameRate, frameNumber)) //if vizModule is active at this frame. Frame rate is an instance variable, frame is from this for loop
      {
        renderStack.push_back(vizModules[i]);
      }
    } // After this for loop, all active vizModules are in the renderStack, in depth order (deepest first)

    //cout << "made it to location 1!" << endl;

    //The canvas itself is used as avirtual background of user set (default black) colored pixels
    for (int index=0; index < renderStack.size(); index++)
    {
      frameNumber = frameNumber_in; // reset frame number because it might have been changed earlier
      if (renderStack[index].still_image == true){
        frameNumber = 0;
      }
      int adjFrameNumber = frameNumber;
      if (!fs::exists(renderStack[index].path + "output" + to_string(frameNumber) + "." + renderStack[index].dataType))
      {
        cout << "Temp message in canvas.cpp; superimpose, file DNE, modulo if case initiating" << endl;
        int filesInDir=0;
        for (auto& p : fs::directory_iterator(renderStack[index].path)) {
          filesInDir++;
        }
        //for 1200 frames in dir, this will be 1200%1200 = 0
        adjFrameNumber = (frameNumber)%filesInDir;
      }

      //cout << "made it to location 2!" << endl;
      //uintmax_t size_img = fs::file_size(renderStack[index].path + "output" + to_string(adjFrameNumber) + "." + renderStack[index].dataType);
      //put on the thread_poo
      //Mat * fg = malloc(size_img);
      //Read the file
      //fg = imread( renderStack[index].path + "output" + to_string(adjFrameNumber) + "." + renderStack[index].dataType, IMREAD_UNCHANGED);//Read the image

      //##############################################################################################################################
      Mat * fg = new Mat(imread(renderStack[index].path + "output" + to_string(adjFrameNumber) + "." + renderStack[index].dataType, IMREAD_UNCHANGED));
      //##############################################################################################################################

      //Check that the file is read successfully
      if(fg->empty()){
         cout << "Error opening foreground file" << endl;
         cout <<  renderStack[index].path + "output" + to_string(adjFrameNumber) + ".png" << endl;
      }
      //cout << "made it to location 3!" << endl;
      ////Initialize a Mat object with 3 channels and width, height
      //cv::Mat img_C3( x, y, CV_8UC3, CV_RGB(1,1,1) );

      //##############################################################################################################################
      Scalar_<uint8_t> bg_bgr;
      Scalar_<uint8_t> fg_bgr;

      vector<int>bounds; //initialize validIndexes as empty
      bounds = computeBounds(renderStack[index], bckg.cols, bckg.rows);
      //cout << "made it to location 4!" << endl;
      //Still need logic to control the positions of the vizModule
      for (int v_index = bounds[0]; v_index < bounds[1]; v_index++){
        for (int h_index = bounds[2]; h_index < bounds[3]; h_index++){
          cout << "stack number " << index << " vstart " << bounds[0] << " vend " << bounds[1] << " hstart " << bounds[2] << " hend " << bounds[3] << endl;
          cout << "vindex " << v_index << " hindex " << h_index  << "fg rows" << fg->rows << " fg cols " << fg->cols  << " number of channels " << fg->channels() <<   endl;
          //Execute only if the foreground alpha channel (4th inde x starting at 1) is greater than 0
          //Access the first pixel [0][0], 4th channel [3] from cv::Mat object
          //This implementation of alpha mixing might not be right btw
          Vec4b bg_channels = bckg.at<Vec4b>(v_index, h_index);
          //Error here in this code
          Vec4b fg_channels;
          Vec3b temp;
          cout << type2str(fg->type()) << endl;
          int fg_y = v_index - bounds[0];
          int fg_x = h_index - bounds[2];
          if (type2str(fg->type()) == "8UC4"){
            fg_channels = fg->at<Vec4b>(fg_x, bg_y);
          } else if (type2str(fg->type()) == "8UC3")
          {
            temp = fg->at<Vec3b>(v_index,h_index);
            fg_channels = Vec4b(temp.val[0], temp.val[1], temp.val[2], 255);
          }
          if (fg_channels.val[3] > 0) //If the foreground's alpha channel is greater than 0
          {
            //Add +1 to channels, since there are 4 channels in a bgr-alpha image
            //Add +3 at the end to get the 4th (indexing starts at 0!) channel

            //https://learnopencv.com/alpha-blending-using-opencv-cpp-python/
            double alpha_weight = fg_channels.val[3]/255.0;
            fg_channels.val[0] = alpha_weight*fg_channels[0]; // B
            fg_channels.val[1] = alpha_weight*fg_channels[1]; // G
            fg_channels.val[2] = alpha_weight*fg_channels[2]; // R

            bg_channels.val[0] = (1-alpha_weight)*bg_channels[0]+fg_channels.val[0]; // B
            bg_channels.val[1] = (1-alpha_weight)*bg_channels[1]+fg_channels.val[1]; // G
            bg_channels.val[2] = (1-alpha_weight)*bg_channels[2]+fg_channels.val[2]; // R
            //Mat A(1,3, CV_8UC4, bg_channels);//I think this line might be useless
            bckg.at<Vec4b>(v_index, h_index)=bg_channels;

          }
        }
      }
      //clean up heap
      delete fg;

    }
    //outside of the for loop which iterates through every active frame in the renderStack
    imwrite("./composite_pngs/composited_img" + to_string(frameNumber) + ".png", bckg);
    cout << "Frame " << to_string(frameNumber) << " superimposed." << endl;
  } catch (exception& e) {
    terminate(); //kill the whole program
  }
}

void vCanvas::renderCanvas(){
  cout << "vCanvas::renderCanvas: Canvas rendering started.  Sorting modules by depth... " << endl;
  int frameRateIn = frameRate;
  //HERE we create an instance variable for this vCanvas which represents the background
  //The color and alpha of this background is set as vector<int> backgroundRGBA
  //I think it is of type cv::Mat

  //synced_stream sync_out; // This must be initialized before the thread_pool object
  thread_pool pool(threads); //initialize a thread_pool for rendering
  // Construct a synced stream that will print to std::cout.

  cout << "Thread creation successful" << endl;

  int frameLength = videoLengthTime * frameRate;
  frameLength = 10731;
  vector<vizModule> vizModulesIn = sortByDepth();   //sort vizModules and store in local variable

  cout << "Depth sorting success!" << endl;


  if (doConversion){
    cout << "Converting files to standard format" << endl;
    for (int n = 0; n < vizModulesIn.size(); n++){
      convertVideoToUsable(vizModulesIn[n]);
    }
    cout << "File conversion to usable and renamed succeeded!" << endl << endl;
  } else {
    cout << "File conversion to usable and renamed bypassed" << endl << endl;
  }


  cout << "vCanvas::renderCanvas: rasterizing background for superimposition" << endl;
  //Include the logic for turning non png type vizModules to usable data
  //This has to be multi-threaded to complete with any appreciable speed
  //thread_pool.submit() might work here?
  //include the logic for renaming arbitrarily named files into something standardized
  //within this conversion process, so that no more work has to be done in this function
  //before pushing files to the pool for superimposition

  //##############################################################################################################################
  //This needs to be come a shiba image
  Mat bg(height, width, CV_8UC4, backgroundRGBA); //Third argument is supposed to be an 8bit length 4 Scalar
  //##############################################################################################################################

  //This forloop checks out 1 frame at a time. Hopefully, don't worry about race conditions
  //After this point, vizModules is not modified, and each thread individually
  //checks if the ordered vizModule objects in vector<vizModule> vizModules is
  //active for the given frame
  cout << "Entering the render loop" << endl << endl;

  for (int frame = 0 ; frame < frameLength; frame++)
  {
    //send a frame number and a time to superimpose.  &sync_out
    //Let the superimpose function decide which vizModule is active, for best performance
    pool.push_task([frame, vizModulesIn, bg, frameRateIn] {superimpose(frame, vizModulesIn, bg, frameRateIn);});
    //I think that I have no choice but to send a lot of parameters to the super impose function and
    //remove it as a member function of the vCanvas class


  }
}

/*
//does not push the render through the thread pool for better debugging
void vCanvas::renderCanvas_1T(){
  cout << "vCanvas::renderCanvas: Canvas rendering started.  Sorting modules by depth... " << endl;
  int frameRateIn = frameRate;
  //HERE we create an instance variable for this vCanvas which represents the background
  //The color and alpha of this background is set as vector<int> backgroundRGBA
  //I think it is of type cv::Mat

  //synced_stream sync_out; // This must be initialized before the thread_pool object
  // Construct a synced stream that will print to std::cout.

  cout << "Thread creation successful" << endl;

  int frameLength = videoLengthTime * frameRate;
  frameLength = 20;
  vector<vizModule> vizModulesIn = sortByDepth();   //sort vizModules and store in local variable

  cout << "Depth sorting success!" << endl;


  if (doConversion){
    cout << "Converting files to standard format" << endl;
    for (int n = 0; n < vizModulesIn.size(); n++){
      convertVideoToUsable(vizModulesIn[n]);
    }
    cout << "File conversion to usable and renamed succeeded!" << endl << endl;
  } else {
    cout << "File conversion to usable and renamed bypassed" << endl << endl;
  }


  cout << "vCanvas::renderCanvas: rasterizing background for superimposition" << endl;
  //Include the logic for turning non png type vizModules to usable data
  //This has to be multi-threaded to complete with any appreciable speed
  //thread_pool.submit() might work here?
  //include the logic for renaming arbitrarily named files into something standardized
  //within this conversion process, so that no more work has to be done in this function
  //before pushing files to the pool for superimposition
  Mat bg(height, width, CV_8UC4, backgroundRGBA); //Third argument is supposed to be an 8bit length 4 Scalar

  //This forloop checks out 1 frame at a time. Hopefully, don't worry about race conditions
  //After this point, vizModules is not modified, and each thread individually
  //checks if the ordered vizModule objects in vector<vizModule> vizModules is
  //active for the given frame
  cout << "Entering the render loop" << endl << endl;

  for (int frame = 0 ; frame < frameLength; frame++)
  {
    //send a frame number and a time to superimpose.  &sync_out
    //Let the superimpose function decide which vizModule is active, for best performance
    superimpose(frame, vizModulesIn, bg, frameRateIn);
    //I think that I have no choice but to send a lot of parameters to the super impose function and
    //remove it as a member function of the vCanvas class

  }
}
*/
