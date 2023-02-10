#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>


using namespace cv;
using std::cout;
using namespace std;

int threshold_value = 0;
int threshold_type = 3;
int const max_value = 255;
int const max_type = 4;
int const max_binary_value = 255;
Mat image, image_gray, binary_image, 
morph_binary_image, morph_ero_di_image, morph_open_close_image;
const char* window_name = "Threshold and Morphology";
const char* trackbar_value = "Thresh";


int morph_size_1 = 0;
int morph_size_2 = 0;
int morph_operator_1 = 0;
int morph_operator_2 = 0;
int const max_operator = 3;
int const max_kernel_size = 21;

double distanceCalculate(double x1, double y1, double x2, double y2) {
    double x = x1 - x2; //calculating number to square in next step
    double y = y1 - y2;
    double dist;

    dist = pow(x, 2) + pow(y, 2);       //calculating Euclidean distance
    dist = sqrt(dist);

    return dist;
}

void Threshold_and_Morphology_Operations(int, void*)
{   

    threshold(image_gray, binary_image, threshold_value, max_binary_value, 3);
    
    /*
    MORPH_ERODE = 0
    MORPH_DILATE = 1
    MORPH_OPEN = 2 
    MORPH_CLOSE = 3
   */
    int operation_1 = morph_operator_1;
    int operation_2 = morph_operator_2;
    Mat element_1 = getStructuringElement(0, Size(2 * morph_size_1 + 1, 2 * morph_size_1 + 1), 
                                        Point(morph_size_1, morph_size_1));
    Mat element_2 = getStructuringElement(0, Size(2 * morph_size_2 + 1, 2 * morph_size_2 + 1),
        Point(morph_size_2, morph_size_2));

    morphologyEx(binary_image, binary_image, operation_1, element_1);
    morphologyEx(binary_image, binary_image, operation_2, element_2);
    imshow(window_name, binary_image);

}

struct elementDistance
{
    int i, j;
    double distance;
};

bool compareDistances(elementDistance i1, elementDistance i2)
{
    return (i1.distance < i2.distance);
}

void getCountours(Mat imgDil, Mat img)
{
    vector<vector<Point>> contours;
    vector<Vec4i> hiearchy;
    vector<Moments> M(contours.size());
    vector<Point> centers;

    findContours(imgDil, contours, hiearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    drawContours(img, contours, -1, Scalar(255, 0, 255), 2);

    vector<Rect> boundRect(contours.size());


    vector<elementDistance> distanceStars;




    for (int i = 0; i < contours.size(); i++) {
        boundRect[i] = boundingRect(contours[i]);
    }

    for (int i = 0; i < contours.size(); i++) {
        Moments M = moments(contours[i]);
        Point center(M.m10 / M.m00, M.m01 / M.m00);
        centers.push_back(center);
    }

    for (int i = 0; i < centers.size(); i++) {
        for (int j = 0; j < centers.size(); j++) {
            if (i != j) {
                double dist = distanceCalculate(centers[i].x, centers[i].y, centers[j].x, centers[j].y);
                distanceStars.push_back({i,j,dist});                
            }
        }
        sort(distanceStars.begin(), distanceStars.end(), compareDistances);

        for (int a = 0; a < distanceStars.size(); a++)
        {
            cout << distanceStars[a].i << ", " << distanceStars[a].j
                << ", " << distanceStars[a].distance << endl;
        }
        vector<Point> four_elements;
        four_elements.push_back(centers[distanceStars[0].i]);
        four_elements.push_back(centers[distanceStars[1].j]);
        four_elements.push_back(centers[distanceStars[2].j]);
        four_elements.push_back(centers[distanceStars[3].j]);

        const Point* pts = (const cv::Point*)Mat(four_elements).data;
        int npts = Mat(four_elements).rows;

        polylines(img, &pts, &npts, 1, true, Scalar(255, 0, 0));
        imshow("cluster stars", img);
        waitKey();

        distanceStars.pop_back();
    }

}






int main(int argc, char** argv)
{
    image = imread("Resources/StarMap.png"); // Load an image
    resize(image, image, Size(), 0.70, 0.70);
    cvtColor(image, image_gray, COLOR_BGR2GRAY); // Convert the image to Gray
    namedWindow(window_name, WINDOW_AUTOSIZE); // Create a window to display results

    createTrackbar(trackbar_value,
        window_name, &threshold_value,
        max_value, Threshold_and_Morphology_Operations); // Create a Trackbar to choose Threshold value 180 işimi gördü
    createTrackbar("Erode-Dilate", 
        window_name, &morph_operator_1, 1, Threshold_and_Morphology_Operations);
    createTrackbar("Kernel1", window_name,
        &morph_size_1, max_kernel_size,
        Threshold_and_Morphology_Operations);
    createTrackbar("Open-Close",
        window_name, &morph_operator_2, 1, Threshold_and_Morphology_Operations);
    createTrackbar("Kernel2", window_name,
        &morph_size_2, max_kernel_size,
        Threshold_and_Morphology_Operations);
    Threshold_and_Morphology_Operations(0, 0);
    waitKey();
    imshow("binary", binary_image);
    waitKey();
	getCountours(binary_image, image);
    imshow("countor", image);
    waitKey();
    return 0;
} 