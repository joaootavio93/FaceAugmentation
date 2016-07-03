#include "stdafx.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

  #define SSTR( x ) dynamic_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str()
using  namespace std;
using namespace cv;

Rect box;
bool drawing_box = false;
struct mousecallbackstruct{
    Mat* src;
    Mat* overlay;
    string windowname;
};
Mat srcoverlay,smallsrcoverlay;




void onMouse(int event, int x, int y, int flags, void* param)  //it seems the only way to use this is by keeping different globals for different windows - meaning you have to set up all thise ahead of time, and keep track of it and not mix/match windows/frames!!  horrible design here.
{
    cout << event;
    mousecallbackstruct mousestruct;
    mousestruct = *((mousecallbackstruct*)param);
    Mat* srcp = mousestruct.src;
    Mat* overlayp = mousestruct.overlay;
    Mat src = *srcp;
    Mat overlay = *overlayp;

    if(!src.data){
        cout <<  "your void * cast didn't work :(\n";
        return;
    }
    switch( event ){
        case CV_EVENT_MOUSEMOVE: 
            if( drawing_box ){
                box.width = x-box.x;
                box.height = y-box.y;
            }
            break;

        case CV_EVENT_LBUTTONDOWN:  //start drawing
            drawing_box = true;
            box = cvRect( x, y, 0, 0 );
            break;
        case CV_EVENT_LBUTTONDBLCLK:  //double click to clear
            drawing_box = false;
            overlay.setTo(cv::Scalar::all(0));  //clear it
            break;
        case CV_EVENT_LBUTTONUP:  //draw what we created with Lbuttondown
            drawing_box = false;
            if( box.width < 0 ){
                box.x += box.width;
                box.width *= -1;
            }
            if( box.height < 0 ){
                box.y += box.height;
                box.height *= -1;
            }
            rectangle( overlay, Point(box.x, box.y), Point(box.x+box.width,box.y+box.height),CV_RGB(100,200,100),4);  //draw rectangle.  You can change this to line or circle or whatever.  Maybe with the Right mouse button.
            break;
    }
}


void iimshow(mousecallbackstruct* mystructp){  //this is where we add the text/drawing created in the mouse handler to the actual image (since mouse handler events do not line up with the drawing events)

    mousecallbackstruct mystruct = *mystructp;  //custom struct made for the mouse callback - very handy for other functions too
    Mat overlay, src;
    Mat* srcp = mystruct.src;
    Mat* overlayp = mystruct.overlay;
    src = *srcp;                                // yeah, yeah, i use 9 lines where I could use 3, so sue me
    overlay = *overlayp;
    string name = mystruct.windowname;
    Mat added,imageROI;

    try{
        //cout << "tch:" << overlay.rows << "," << src.rows << ";" <<  overlay.cols <<  "," << src.cols << ";" <<  src.channels() <<  "," << overlay.channels() <<"," <<  src.type() <<  "," << overlay.type() << "\n";
        if(overlay.data && overlay.rows == src.rows && overlay.cols == src.cols && overlay.channels() == src.channels()){  //basic error checking
            add(src,overlay,added);
        }else{
           //try to resize it
            imageROI= overlay(Rect(0,0,src.cols,src.rows));
            add(src,imageROI,added);
        }

        imshow(name,added);// the actual draw moment

    }catch(...){  //if resize didn't work then this should catch it
        cout << "Error.  Mismatch:" << overlay.rows << "," << src.rows << ";" <<  overlay.cols <<  "," << src.cols << ";" <<  src.channels() <<  "," << overlay.channels() <<"," <<  src.type() <<  "," << overlay.type() << "\n";
        imshow(name + "overlay",overlay);
        imshow(name+"source",src);
    }

}

int _tmain(int argc, _TCHAR* argv[])
{
     VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()) { // check if we succeeded
        cout << "NO camera found \n";
        return -1;
    }

    Mat src,smallsrc,overlay;
    cap >> src;  //grab 1 frame to build our preliminary Mats and overlays

    srcoverlay.create(src.rows,src.cols,src.type());  //create overlays
    smallsrcoverlay.create(src.rows,src.cols,src.type());
    srcoverlay.setTo(cv::Scalar::all(0));  //clear it
    smallsrcoverlay.setTo(cv::Scalar::all(0));  //clear it

    namedWindow( "smallsrc", CV_WINDOW_AUTOSIZE );
    namedWindow( "source", CV_WINDOW_AUTOSIZE );  //these must be created early for the setmousecallback, AND you have to know what Mats will be using them and not switch them around :(
    moveWindow("smallsrc",2000,100);  //create a small original capture off to the side of screen

    ////////////// for each window/mat that uses a mouse handler, you must create one of these structures for it and pass it into the mouse handler
    mousecallbackstruct srcmousestruct,smallsrcmousestruct;  //these get passed into the mouse callback function.  Hopefully they update their contents automatically for the callback?  :(
    srcmousestruct.overlay = &srcoverlay;  //fill our custom struct
    srcmousestruct.src = &src;
    srcmousestruct.windowname = "source";

    smallsrcmousestruct.overlay = &smallsrcoverlay;  //the small window
    smallsrcmousestruct.src = &smallsrc;
    smallsrcmousestruct.windowname = "smallsrc";

    setMouseCallback(smallsrcmousestruct.windowname, onMouse, (void*)&smallsrcmousestruct); //the actual 'set mouse callback' call
    setMouseCallback(srcmousestruct.windowname, onMouse, (void*)&srcmousestruct);

    for(;;){  //main loop
      /// Load an image
      cap >> src;

      if( !src.data )
      { return -1; }

      resize(src,smallsrc,Size(),.5,.5);  //smaller scale window of original

      overlay = *srcmousestruct.overlay;
      src = *srcmousestruct.src;

      iimshow(&srcmousestruct);  //my imshow replacement.  uses structs
      iimshow(&smallsrcmousestruct);

      if(waitKey(30) == 27) cin.get();  //esc pauses
    }
    cin.get();
    return 0;
}