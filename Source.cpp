#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/features2d.hpp"
#include <vector>
#include <array>
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.15f;


void alignImages(Mat& im1, Mat& im2, Mat& im1Reg, Mat& h)

{

    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cvtColor(im1, im1Gray, COLOR_BGR2GRAY);
    cvtColor(im2, im2Gray, COLOR_BGR2GRAY);

    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(descriptors1, descriptors2, matches, Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin() + numGoodMatches, matches.end());


    // Draw top matches
    Mat imMatches;
    drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
    imwrite("matches.jpg", imMatches);


    // Extract location of good matches
    std::vector<Point2f> points1, points2;

    for (size_t i = 0; i < matches.size(); i++)
    {
        points1.push_back(keypoints1[matches[i].queryIdx].pt);
        points2.push_back(keypoints2[matches[i].trainIdx].pt);
    }

    // Find homography
    h = findHomography(points1, points2, RANSAC);

    // Use homography to warp image
    warpPerspective(im1, im1Reg, h, im2.size());

}


int main(int argc, char** argv)
{
    // Read reference image
    string refFilename("C:\\CODE\\orb\\3.jpg");
    cout << "Reading reference image : " << refFilename << endl;
    Mat imReference = imread(refFilename);


    // Read image to be aligned
    string imFilename("C:\\CODE\\orb\\1.jpg");
    cout << "Reading image to align : " << imFilename << endl;
    Mat im = imread(imFilename);


    // Registered image will be resotred in imReg. 
    // The estimated homography will be stored in h. 
    Mat imReg, h;

    // Align images
    cout << "Aligning images ..." << endl;
    alignImages(im, imReference, imReg, h);

    // Write aligned image to disk. 
    string outFilename("aligned.jpg");
    cout << "Saving aligned image : " << outFilename << endl;
    imwrite(outFilename, imReg);

    // Print estimated homography
    cout << "Estimated homography : \n" << h << endl;

    Mat dst;
    Mat gray;
    Mat gray_1;
    Mat rgb = imread("C:\\CODE\\orb\\aligned.jpg");//path for test image
    Mat rgb_1 = imread("C:\\CODE\\orb\\3.jpg");//path for reference image
    cvtColor(rgb, gray, COLOR_RGB2GRAY);
    cvtColor(rgb_1, gray_1, COLOR_RGB2GRAY);
    equalizeHist(gray, dst);
    Mat img_bw = gray > 138;
    Mat img_bw_1 = gray_1 > 138;
    Mat result = img_bw_1 - img_bw;
    Mat result_1;
    //comment below out for display
    //namedWindow("Display window1", WINDOW_NORMAL);
    //imshow("Display window1", result);
    //namedWindow("Display window2", WINDOW_NORMAL);
    //imshow("Display window2", img_bw);
    //namedWindow("Display window3", WINDOW_NORMAL);
    //imshow("Display window3", img_bw_1);
    imwrite("im_gray.jpg", gray);
    imwrite("im_gray_1.jpg", gray_1);
    imwrite("im_bw.jpg", img_bw);
    imwrite("im_bw_1.jpg", img_bw_1);

    fastNlMeansDenoising(result, result_1, 40.0, 29, 51)//tweak parameters wrt final image details 
    
    imwrite("Defect.jpg", result_1);
    waitKey(0);
    return 0;
}
