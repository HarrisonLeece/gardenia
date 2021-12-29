#include <iostream>
#include <filesystem>
#include <experimental/filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "thread_pool.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

class vizModule{
  public:
    //Coordinates of the module relative to the bottom left corner of a canvas
    int x=0; //Positive is right
    int y=0; //Positive is up
    int sizex=0;
    int sizey=0;
    int centerX = 0;
    int centerY = 0;

    float rotation=0.0; //Not functional yet
    int depth=1; //The greater the number, the deeper into the screem (bottom of the 'pile') the module is
    int anchor=0; //0=center, 1 = upper left corner, 2 = bottom left cortner, 3 = bottom right corner, 4 = upper right corner

    //type, for example png, WebM, mp4, tiff
    //makes it easier if we match this name to the file extension
    string dataType = "png";
    //Path
    string path = "./input/"; // very crappy default folder path.  Include the / at the end

    string units = "frames";

    //////Timiing features
    int delay = 0; //0 = no delay to start rendering the module
    int cycles = 0; //1 = render this module once, do not repeat. 0=repeat until all other streams have terminated

    void calculateCenter();
    void setPosition(int anch, int xIn, int yIn, int depthIn, float rotate);
    void setModSize(int xSize, int ySize);
    void addPath(string pathToInput, string typeIn);
    void setUnits(string newUnit);
    void setTiming(int del, int cyc);

    bool isActive(int fps, int frame);


};

void vizModule::calculateCenter(){
  centerX = sizex/2; //close enough is fine
  centerY = sizey/2;
}

void vizModule::setPosition(int anch, int xIn, int yIn, int depthIn, float rotate = 0.0) {
  assert(anchor >= 0 && anchor <= 4);
  anchor = anch;
  x = xIn;
  y = yIn;
  depth = depthIn;
  rotation = rotate;
  //calculateCenter(); //Come back to this when I flesh out rotations
}

void vizModule::setModSize(int xSize, int ySize){
  sizex = xSize;
  sizey = ySize;
  calculateCenter(); // Call this function any time size or roation is changed
}

void vizModule::addPath(string pathToInput, string typeIn) {
  assert(typeIn == "png" || typeIn == "mp4" || typeIn == "webm" || typeIn == "tiff"); //check that type input is valid
  assert(true); //placeholder for checking for a valid filepath
  path = pathToInput;
  dataType = typeIn;
}

void vizModule::setUnits(string newUnit) {
  assert(newUnit == "frames" || newUnit == "times");
  units = newUnit;
}

void vizModule::setTiming(int del, int cyc){
  delay = del;
  cycles = cyc;
}

bool vizModule::isActive(int fps, int frame) {
  bool isActiveBool;
  float timer = frame/(float(fps)); //Have to cast to float or else the division will cast to int.  SMH c++
  if (units == "frames") // if frame is valid for the vizModule
  {
    isActiveBool = true; //not done
  } else {
    isActiveBool = true; //not done
  }
  return isActiveBool;//Always returns true with current implementation
}
