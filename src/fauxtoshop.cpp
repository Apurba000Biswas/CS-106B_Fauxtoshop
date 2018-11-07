/* fauxtoshop.cpp
 * Edited by Apurba Biswas
 * references :
 *  http://web.stanford.edu/class/archive/cs/cs106b/cs106b.1172//handouts/assignments.html
 *  https://stanford.edu/~stepp/cppdoc/
 * course: CS106B (Programing abstruction) - Assignment 01
 *
 * This file will implement 4 diffrent image editing flaviour
 */

#include <iostream>
#include "console.h"
#include "gwindow.h"
#include "grid.h"
#include "simpio.h"
#include "strlib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "math.h" //for sqrt and exp in the optional Gaussian kernel
#include "random.h"
#include<math.h>
#include "vector.h"
#include <cctype>
using namespace std;

// constatnts
static const int    WHITE = 0xFFFFFF;
static const int    BLACK = 0x000000;
static const int    GREEN = 0x00FF00;
static const double PI    = 3.14159265;

// prototypes
void     doFauxtoshop(GWindow &gw, GBufferedImage &img);
bool     openImage(GBufferedImage &img, string imgFileName);
string   getFileName();
int      getIntegerInRange(const int low, const int high, const string keyMessage, const string message);
void     promptUser(const string keyMessage);
void     doScatter(Grid<int> &original, Grid<int> &newImageData);
void     triggerOperation(const int choice, GBufferedImage &img);
int      getPixelWithinRadius(const int radius, const int row, const int col, Grid<int> &original);
void     doEdgeDetection(Grid<int> &original, Grid<int> &newImageData);
int      getDifferenceOfPixels(const int pRed, const int pGreen, const int pBlue, const int givenPixel);
int      getMaxNeighborDistance(Grid<int> &original , const int row, const int col,
                                const int red, const int green, const int blue);
void     doGreenScreenEfect(Grid<int> &background, Grid<int> &resultantImageData);
void     openSecondImage(GBufferedImage &img);
void     getLocation(int &row, int &col);
void     getLocationFromEnterdString(int &row, int &col, string location);
void     makeImage(Grid<int> &background, Grid<int> &resultantImageData, Grid<int> &sticker,
               const int thresohold, const int locRow, const int locCol);
void     doCompareWithAontherImage(GBufferedImage &img);

bool     openImageFromFilename(GBufferedImage& img, string filename);
bool 	 saveImageToFilename(const GBufferedImage &img, string filename);
void     getMouseClickLocation(int &row, int &col);


/* STARTER CODE FUNCTION - DO NOT EDIT
 *
 * This main simply declares a GWindow and a GBufferedImage for use
 * throughout the program. By asking you not to edit this function,
 * we are enforcing that the GWindow have a lifespan that spans the
 * entire duration of execution (trying to have more than one GWindow,
 * and/or GWindow(s) that go in and out of scope, can cause program
 * crashes).
 */
int main() {
    GWindow gw;
    gw.setTitle("Fauxtoshop");
    gw.setVisible(true);
    GBufferedImage img;
    doFauxtoshop(gw, img);
    return 0;
}

/** doFauxtoshop() function
 * This method dose the whole operation of editing images and controls the
 * operation.Offer four different flavour for image editing
 */
void doFauxtoshop(GWindow &gw, GBufferedImage &img) {
    string fileName;
    string newfileName;
    int choice;
    while(true){
        cout << "Welcome to Fauxtoshop!" << endl;
        cout << "Enter name of image file to open (or blank to qiut) : ";
        fileName = getFileName();
        if(!openImage(img, fileName)){
            break;
        }
        gw.setCanvasSize(img.getWidth(), img.getHeight());
        gw.add(&img,0,0);

        cout << "Which image filter would you like to apply?" << endl;
        choice = getIntegerInRange(1, 4, "choice", "Your choice is: ");
        triggerOperation(choice, img);

        cout << "Enter file name to save image(or blank to skip saving) :";
        newfileName = getFileName();
        if(newfileName != ""){
            saveImageToFilename(img, newfileName);
        }
    }
    cout << "good bye" << endl;
    gw.clear();
}

/** triggerOperation() function
 * This function swicth the operation user is interested now
 * and after performing the operation it add the newly created image
 * in the graphics window
 */
void triggerOperation(const int choice, GBufferedImage &img){
    Grid<int> original = img.toGrid();
    Grid<int> newImageData(original.numRows(), original.numCols());
    switch(choice){
        case 1:
            doScatter(original, newImageData);
            break;
        case 2:
            doEdgeDetection(original, newImageData);
            break;
        case 3:
            doGreenScreenEfect(original, newImageData);
            break;
        case 4:
            doCompareWithAontherImage(img);
            break;
        default:
          cout << "This is not valid choice" << endl;
            break;
    }
    if(choice == 4){
        img.fromGrid(original);
    }else{
        img.fromGrid(newImageData);
    }
}

/** doCompareWithAontherImage() function
 * This function impelements the comparing two images opeartion by taking another image
 * file name from user
 */
void doCompareWithAontherImage(GBufferedImage &img){
    cout << "Now choose another image to compare to : " << endl;
    GBufferedImage newImage;
    openSecondImage(newImage);
    int count = img.countDiffPixels(newImage);
    if(count == 0){
        cout << "These images are the same!" << endl;
    }else{
        cout << "These images differ in " << count << " pixel locations!" << endl;
    }
}
/** doGreenScreenEfect() function
 * This function implements the green screen efect by taking another image as a sticker
 * then asks user to enter location then place the sticker in that location onto the
 * background image
 */
void doGreenScreenEfect(Grid<int> &background, Grid<int> &resultantImageData){
    GBufferedImage newImg;
    openSecondImage(newImg);
    Grid<int> sticker = newImg.toGrid();

    cout << "Now ";
    int thresohold = getIntegerInRange(1, 10000, "threshold", "choose a tolerance threshold: ");
    int locRow , locCol;
    getLocation(locRow, locCol);

    makeImage(background, resultantImageData, sticker, thresohold, locRow, locCol);
}

/** makeImage() function
 * This fucntion actually implements the algorithm of joining pixels
 * which produce the expected image
 */
void makeImage(Grid<int> &background, Grid<int> &resultantImageData, Grid<int> &sticker,
               const int thresohold, const int locRow, const int locCol){
    int gRed,gGreen,gBlue;
    GBufferedImage::getRedGreenBlue(GREEN, gRed, gGreen, gBlue);

    int dis;
    int sCol = 0, sRow = 0;
    bool key = false;

    for(int row = 0; row < background.numRows(); row++){
        for(int col = 0; col < background.numCols(); col++){
            if(locRow <= row && locCol <= col){
                if(sticker.inBounds(sRow, sCol)){
                    dis = getDifferenceOfPixels(gRed, gGreen, gBlue, sticker[sRow][sCol]);
                    if(dis > thresohold){
                        resultantImageData[row][col] = sticker[sRow][sCol];
                     }else{
                        resultantImageData[row][col] = background[row][col];
                     }
                sCol ++;
                }else{
                    resultantImageData[row][col] = background[row][col];
                }
                if(!key)key = true;
            }else{
                resultantImageData[row][col] = background[row][col];
                if(key)key = false;
            }
        }
        if(key)sRow ++;
        sCol = 0;
    }
}
/** getLocation() function
 * This function asks user to enter location to palce the sticker on the background image
 * it also gives two option one is by typing the location and other one is by clicking
 * the mouse on the background image. After retreiving the location it retuns it.
 */
void getLocation(int &row, int &col){
    cout << "Enter location to place image as (row, col) or (or blank to use mouse) : ";
    string location;
    getline(cin, location);
    if(location != ""){
        getLocationFromEnterdString(row, col, location);
    }else{
        cout << "Now click the background image to place new image: " << endl;;
        getMouseClickLocation(row, col);
        cout << "You choose (" << row << "," << col << ")" << endl;
    }
}
/** getLocationFromEnterdString() function
 * It makes location from a string whic user typed. it forms row and colum value
 * from the string.
 */
void getLocationFromEnterdString(int &row, int &col, string location){
    string sRow, sCol;
    int midlePoint = location.find(",");
    for(int i=0; i< location.length(); i++){
        if(i < midlePoint){
            if(isdigit(location.at(i))){
                sRow += location.at(i);
            }
        }else{
            if(isdigit(location.at(i))){
                sCol += location.at(i);
            }
        }
    }
    row = stringToInteger(sRow);
    col = stringToInteger(sCol);
}

/** openSecondImage() function
 * This function just asks user to give a valid name of image
 * then opens it
 */
void openSecondImage(GBufferedImage &img){
    string imageFileName;
    while(true){
        cout << "Enter name of image file to open: ";
        imageFileName = getFileName();
        if(openImage(img, imageFileName)){
            break;
        }
    }
}

/** doEdgeDetection() function
 * This function implements the edge Detection operation and
 * form a new image for that operation
 */
void doEdgeDetection(Grid<int> &original, Grid<int> &newImageData){
    int threshold = getIntegerInRange(1, 10000, "threshold", "Enter threshold for edge detection: ");
    int max, pixel, red, green, blue;

    for(int row = 0; row < original.numRows(); row++){
        for(int col = 0; col < original.numCols(); col++){
            pixel = original[row][col];
            GBufferedImage::getRedGreenBlue(pixel, red, green, blue);

            max = getMaxNeighborDistance(original, row, col, red, green, blue);
            if(max > threshold){
                newImageData[row][col] = BLACK;
            }else{
                newImageData[row][col] = WHITE;
            }
        }
    }
}

/** getMaxNeighborDistance() function
 * this function looks all the neighbor of the current pixel and
 * calculate the distance for each neighbor pixels and findout the
 * maximum distance from the current pixel and returns it
 */
int getMaxNeighborDistance(Grid<int> &original , const int row, const int col,
                           const int red,const int green,const int blue){
    int maximumDiff = -111;
    for(int i=-1; i<2; i++){
        for(int j=-1; j<2; j++){
            if(original.inBounds(row+i, col+j)){
                int diff = getDifferenceOfPixels(red, green, blue,original[row+i][col+j] );
                if(diff > maximumDiff){
                    maximumDiff = diff;
                }
            }
        }
    }
    return maximumDiff;
}

/** getDifferenceOfPixels() function
 * This function compare two pixels individual RGB color
 * pRed, pGreen, pBlue indicated current pixel and
 * nRed, nGreen, nBlue indicated neighbor pixel and finds the distance
 * between two pixels and then returns it
 */
int getDifferenceOfPixels(const int pRed, const int pGreen, const int pBlue, const int givenPixel){
    int nRed, nGreen, nBlue;
    GBufferedImage::getRedGreenBlue(givenPixel, nRed, nGreen, nBlue);

    int maximum = max(abs(pRed - nRed), abs(pGreen - nGreen));
    maximum = max(maximum, abs(pBlue - nBlue));

    return maximum;
}

/** doScatter() function
 * its implement the scattering operation by taking the degree of scatter from user and
 * make new image as expected output
 */
void doScatter(Grid<int> &original, Grid<int> &newImageData){
    int radius = getIntegerInRange(0, 100, "degree", "Enter degree of scatter[0-100]: ");
    int numOfRows = original.numRows();
    int numOfCols = original.numCols();

    for(int row = 0; row < numOfRows; row++){
        for(int col = 0; col < numOfCols; col ++){
            newImageData[row][col] = getPixelWithinRadius(radius, row, col, original);
        }
    }
}
/** getPixelWithinRadius() function
 * This function returns a valid pixel from original
 * image, its also check if the quering pixel is out of index
 * of the original image
 */
int getPixelWithinRadius(const int radius, const int row, const int col, Grid<int> &original){
    while (true) {
        int colNum = randomInteger(col - radius, col + radius);
        if(original.inBounds(row, colNum)){
            return original[row][colNum];
        }
    }
}

/** getIntegerInRange() function
 * when the image is opend in the graphics window then this function gets triggered
 * asks for choice, if user enter valid choice it returns that choice if user type
 * invalid choice it asks again for valid choice
 */
int getIntegerInRange(const int low, const int high, const string keyMessage, const string message){
    int value;
    while(true){
        if(keyMessage == "choice" ){
            cout << "    1 - Scatter" << endl;
            cout << "    2 - Edge detection" << endl;
            cout << "    3 - Green screen with another image" << endl;
            cout << "    4 - Compare image with another image" << endl;
        }
        value = getInteger(message);
        if(value <= high && value >= low){
            return value;
        }
        cout << "Invalid " << keyMessage << " . try again!"<< endl;
    }
}

/** openImage() function
 * Open image file in the graphics window whats was typed by user and return true if user did not enter empty.
 * if user type invalid file name then it asks again to type a valid image file name
 * and if user dosent type any file name at all(by pressing ENTER) then the function returns false
 */
bool openImage(GBufferedImage &img, string imgFileName){
    while (imgFileName != "") {
        if(openImageFromFilename(img, imgFileName)){
            cout << "opening image file, may take a minitue..." << endl;
            return true;
        }else{
            cout << "Invalid Image file name! try again: ";
            imgFileName = getFileName();
        }
    }
    return false;
}

/** getFileName() method
 * This method asks user for enter file name
 * and return a file name
 */
string getFileName(){
    string imgFileName = "";
    getline(cin, imgFileName);
    return imgFileName;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 * Attempts to open the image file 'filename'.
 * This function returns true when the image file was successfully
 * opened and the 'img' object now contains that image, otherwise it
 * returns false.
 */
bool openImageFromFilename(GBufferedImage& img, string filename) {
    try { img.load(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 * Attempts to save the image file to 'filename'.
 * This function returns true when the image was successfully saved
 * to the file specified, otherwise it returns false.
 */
bool saveImageToFilename(const GBufferedImage &img, string filename) {
    try { img.save(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 * Waits for a mouse click in the GWindow and reports click location.
 * When this function returns, row and col are set to the row and
 * column where a mouse click was detected.
 */
void getMouseClickLocation(int &row, int &col) {
    GMouseEvent me;
    do {
        me = getNextEvent(MOUSE_EVENT);
    } while (me.getEventType() != MOUSE_CLICKED);
    row = me.getY();
    col = me.getX();
}
