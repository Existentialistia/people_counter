
#include<iostream>
#include<conio.h>
#include "blob.h"

using namespace std;
using namespace cv;

const Scalar SCALAR_BLACK = Scalar(0.0, 0.0, 0.0);
const Scalar SCALAR_WHITE = Scalar(255.0, 255.0, 255.0);
const Scalar SCALAR_YELLOW = Scalar(0.0, 255.0, 255.0);
const Scalar SCALAR_GREEN = Scalar(0.0, 200.0, 0.0);
const Scalar SCALAR_RED = Scalar(0.0, 0.0, 255.0);
const string filename = "video.mp4";  //set example


// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs);
void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex);
void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs);
double distanceBetweenPoints(Point point1, Point point2);
void drawAndShowContours(Size imageSize, vector<Blob> blobs, string strImageName);
bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &inCount,int &outCount);
void drawCountOnImage(int &inCount,int &outCount, Mat &imgFrame2Copy);
bool countflag;

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {

    VideoCapture capVideo;

    Mat imgFrame1;
    Mat imgFrame2;

    vector<Blob> blobs;

    Point crossingLine[2];

    int outCount=0;
    int inCount=0;

    capVideo.open(filename);

    if (!capVideo.isOpened()) {
        cout << "error reading video file" << endl << endl;
        _getch();
        return(0);
    }

    if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
        cout << "error: video file must have at least two frames";
        _getch();
        return(0);
    }

    capVideo.read(imgFrame1);
    capVideo.read(imgFrame2);

    int intHorizontalLinePosition = round((double)imgFrame1.rows * 0.75);

    crossingLine[0].x = intHorizontalLinePosition;
    crossingLine[0].y = 0;

    crossingLine[1].x = intHorizontalLinePosition;
    crossingLine[1].y = imgFrame1.cols - 1;

    char chCheckForEscKey = 0;

    bool blnFirstFrame = true;

    int frameCount = 2;

    while (capVideo.isOpened() && chCheckForEscKey != 27) {

        vector<Blob> currentFrameBlobs;

        Mat imgFrame1Copy = imgFrame1.clone();
        Mat imgFrame2Copy = imgFrame2.clone();

        Mat imgDifference;
        Mat imgThresh;

        cvtColor(imgFrame1Copy, imgFrame1Copy, 10);
        cvtColor(imgFrame2Copy, imgFrame2Copy, 10);

        GaussianBlur(imgFrame1Copy, imgFrame1Copy, Size(7, 7), 0);
        GaussianBlur(imgFrame2Copy, imgFrame2Copy, Size(7, 7), 0);

        absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

        threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);

        Mat structuringElement5x5 = getStructuringElement(MORPH_RECT, Size(5, 5));

        for (int i = 0; i < 2; i++) {
            dilate(imgThresh, imgThresh, structuringElement5x5);
            dilate(imgThresh, imgThresh, structuringElement5x5);
            erode(imgThresh, imgThresh, structuringElement5x5);
        }

        Mat imgThreshCopy = imgThresh.clone();

        vector<vector<Point> > contours;

        findContours(imgThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        vector<vector<Point> > convexHulls(contours.size());

        for (unsigned int i = 0; i < contours.size(); i++) {
            convexHull(contours[i], convexHulls[i]);
        }

        for (auto &convexHull : convexHulls) {
            Blob possibleBlob(convexHull);
            if (possibleBlob.currentBoundingRect.area() > 10 &&
                possibleBlob.dblCurrentAspectRatio > 0.4 &&
                possibleBlob.dblCurrentAspectRatio < 2 &&
                possibleBlob.currentBoundingRect.width > 20 &&
                possibleBlob.currentBoundingRect.height > 20 &&
                possibleBlob.dblCurrentDiagonalSize > 40.0 &&
                (contourArea(possibleBlob.currentContour) / (double)possibleBlob.currentBoundingRect.area()) > 0.7) {
                currentFrameBlobs.push_back(possibleBlob);
                }
        }

        if (blnFirstFrame == true) {
            for (auto &currentFrameBlob : currentFrameBlobs) {
                blobs.push_back(currentFrameBlob);
            }
        } else {
            matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
        }

        drawAndShowContours(imgThresh.size(), blobs, "imgBlobs");

        imgFrame2Copy = imgFrame2.clone();

        bool blnAtLeastOneBlobCrossedTheLine = checkIfBlobsCrossedTheLine(blobs, intHorizontalLinePosition, inCount,outCount);

        if (blnAtLeastOneBlobCrossedTheLine == true) {
            line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_GREEN, 2);
            countflag = false;
        } else {
            line(imgFrame2Copy, crossingLine[0], crossingLine[1], SCALAR_RED, 2);
            countflag = true;
        }

        drawCountOnImage (inCount,outCount, imgFrame2Copy);

        imshow("OutputFrame", imgFrame2Copy);

        currentFrameBlobs.clear();

        imgFrame1 = imgFrame2.clone();

        if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) {
            capVideo.read(imgFrame2);
        } else {
            cout << "end of video\n";
            break;
        }

        blnFirstFrame = false;
        frameCount++;
        chCheckForEscKey = waitKey(1);
    }

    if (chCheckForEscKey != 27) {
        waitKey(0);
    }

    return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlobsToExistingBlobs(vector<Blob> &existingBlobs, vector<Blob> &currentFrameBlobs) {

    for (auto &existingBlob : existingBlobs) {

        existingBlob.blnCurrentMatchFoundOrNewBlob = false;

        existingBlob.predictNextPosition();
    }

    for (auto &currentFrameBlob : currentFrameBlobs) {

        int intIndexOfLeastDistance = 0;
        double dblLeastDistance = 100000.0;

        for (unsigned int i = 0; i < existingBlobs.size(); i++) {

            if (existingBlobs[i].blnStillBeingTracked == true) {

                double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);

                if (dblDistance < dblLeastDistance) {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;
                }
            }
        }

        if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 0.8) {
            addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
        }
        else {
            addNewBlob(currentFrameBlob, existingBlobs);
        }

    }

    for (auto &existingBlob : existingBlobs) {

        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {
            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
        }

        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
            existingBlob.blnStillBeingTracked = false;
        }

    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, vector<Blob> &existingBlobs, int &intIndex) {

    existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;
    existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());
    existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
    existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;
    existingBlobs[intIndex].blnStillBeingTracked = true;
    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, vector<Blob> &existingBlobs) {

    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
    existingBlobs.push_back(currentFrameBlob);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(Point point1, Point point2) {

    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);
    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(Size imageSize, vector<Blob> blobs, string strImageName) {

    Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    vector<vector<Point> > contours;

    for (auto &blob : blobs) {
        if (blob.blnStillBeingTracked == true) {
            contours.push_back(blob.currentContour);
        }
    }
    drawContours(image, contours, -1, SCALAR_WHITE, -1);
    imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool checkIfBlobsCrossedTheLine(vector<Blob> &blobs, int &intHorizontalLinePosition, int &inCount,int &outCount) {
    bool blnAtLeastOneBlobCrossedTheLine = false;

    for (auto blob : blobs) {

        if (blob.blnStillBeingTracked == true && blob.centerPositions.size() >= 2.5) {
            int prevFrameIndex = (int)blob.centerPositions.size() - 2;
            int currFrameIndex = (int)blob.centerPositions.size() - 1;

            if (blob.centerPositions[prevFrameIndex].x > intHorizontalLinePosition && blob.centerPositions[currFrameIndex].x <= intHorizontalLinePosition) {
                outCount++;
                blnAtLeastOneBlobCrossedTheLine = true;
            }
               else if(blob.centerPositions[prevFrameIndex].x < intHorizontalLinePosition && blob.centerPositions[currFrameIndex].x >= intHorizontalLinePosition && countflag==true) {
                    inCount++;
                    blnAtLeastOneBlobCrossedTheLine = true;
            }
        }

    }
    return blnAtLeastOneBlobCrossedTheLine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCountOnImage(int &inCount,int &outCount, Mat &imgFrame2Copy) {
    Point ptTextBottomLeftPosition,ptTextBottomRightPosition;

    ptTextBottomLeftPosition.x = imgFrame2Copy.rows * 0.01;
    ptTextBottomLeftPosition.y = imgFrame2Copy.cols *0.7;

    ptTextBottomRightPosition.x = imgFrame2Copy.rows * 1;
    ptTextBottomRightPosition.y = imgFrame2Copy.cols * 0.7;

    putText(imgFrame2Copy, " OUT: " + to_string(outCount), ptTextBottomLeftPosition, CV_FONT_HERSHEY_SIMPLEX, 0.8, SCALAR_GREEN,2);
    putText(imgFrame2Copy," IN: " + to_string(inCount), ptTextBottomRightPosition, CV_FONT_HERSHEY_SIMPLEX, 0.8, SCALAR_GREEN, 2);
}
