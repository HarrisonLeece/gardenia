#include <iostream>
#include <filesystem>
#include <experimental/filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "thread_pool.hpp"

using namespace std;

class vizModule{
  public:
    //Coordinates of the module relative to the bottom left corner of a canvas
    int x=0; //ONLY SET WITH FUNCTION; Positive is right // the position set by the user;
    int y=0; //ONLY SET WITH FUNCTION; Positive is up // the position set by the user
    int ul_y;
    int ul_x;
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

    bool still_image = false; 

    //////Timiing features
    int delay = 0; //0 = no delay to start rendering the module
    int cycles = 0; //1 = render this module once, do not repeat. 0=repeat until all other streams have terminated

    //flow control
    bool position_set = false;
    bool size_set = false;

    void calculateCenter();
    void setPosition(int anch, int xIn, int yIn, int depthIn, float rotate);
    void setModSize(int xSize, int ySize);
    void addPath(string pathToInput, string typeIn);
    void setUnits(string newUnit);
    void setTiming(int del, int cyc);
    void calc_ul_coord(void);

    bool isActive(int fps, int frame);


};

void vizModule::calculateCenter(){
  centerX = sizex/2; //close enough is fine
  centerY = sizey/2;
}

void vizModule::setPosition(int anch, int xIn, int yIn, int depthIn, float rotate = 0.0) {
  position_set = true;
  assert(anchor >= 0 && anchor <= 4);
  anchor = anch;
  x = xIn;
  y = yIn;
  depth = depthIn;
  rotation = rotate;
  //calculateCenter(); //Come back to this when I flesh out rotations
  if(size_set)
  {
    calc_ul_coord();
  }
}

void vizModule::setModSize(int xSize, int ySize){
  size_set = true;
  sizex = xSize;
  sizey = ySize;
  calculateCenter(); // Call this function any time size or roation is changed
  if (position_set) //Only execute if position has already been set
  {
      calc_ul_coord(); //
  }

}

void vizModule::calc_ul_coord(void) {
  //Transform the point of reference to the upper left corner of the module
  if (anchor == 0) //the reference location for x,y position is the center
  {
    ul_y = y+centerY;
    ul_x = x-centerX;

  } else if (anchor == 1) //upper left corner
  {
    ul_y = y;//Good to go!
  } else if (anchor == 2) //Bottom left corner
  {
    ul_y = y + sizey;
  } else if (anchor == 3) // bottom right conrner
  {
    ul_x = x - sizex;
    ul_y = y + sizey;
  } else if (anchor == 4) //module.anchor == 4 -- upper right corner
  {
    ul_x = x - sizex;
  }
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
