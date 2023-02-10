#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <algorithm>


using namespace cv;
using std::cout;
using namespace std;

Mat image, image_gray, binary_image;
int threshold_value = 204; //optimum threshold
int const max_value = 255;
int const max_binary_value = 255;
const char* window_name = "Threshold and Morphology";
const char* trackbar_value = "Thresh";
const char* first_morp_bar = "Ero-Di"; //Erode-Dilate
const char* second_morp_bar = "Open-Close";
const char* kernel_1 = "Kernel1"; //Unforget 2n+1
const char* kernel_2 = "Kernel2";

int kernel_size_1 = 1;// 2n+1=3 optimum structing element size
int kernel_size_2 = 0;
int morph_operator_1 = 1; // Dilate is perfect solving in this case after thresold
int morph_operator_2 = 0;
int const max_kernel_size = 21;

double distanceCalculate(double x1, double y1, double x2, double y2) {
    double x = x1 - x2; 
    double y = y1 - y2;
    double dist;

    dist = pow(x, 2) + pow(y, 2);//calculating Euclidean distance
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
    Mat element_1 = getStructuringElement(0, Size(2 * kernel_size_1 + 1, 2 * kernel_size_1 + 1),
        Point(kernel_size_1, kernel_size_1));
    Mat element_2 = getStructuringElement(0, Size(2 * kernel_size_2 + 1, 2 * kernel_size_2 + 1),
        Point(kernel_size_2, kernel_size_2));

    morphologyEx(binary_image, binary_image, operation_1, element_1);
    morphologyEx(binary_image, binary_image, operation_2, element_2);
    imshow(window_name, binary_image);

}

struct elementDistance // To be used when keeping the distances of the stars from each other
{
    int i, j;
    double distance;
};

bool compareDistances(elementDistance i1, elementDistance i2) // To be used while sorting measured distances
{
    return (i1.distance < i2.distance);
}

bool compareCoordinatesX(Point i1, Point i2) // To be used while sorting coordinates of centers of stars
{
    return i1.x * i1.x + i1.y * i1.y < i2.x* i2.x + i2.y * i2.y;
}


void getCountoursAndClusterStars(Mat imgDil, Mat img)
{
    vector<vector<Point>> contours;
    vector<Vec4i> hiearchy; 
    vector<Moments> M(contours.size());
    vector<Point> centers;
    vector<Point> four_stars_after_calculate_distance;

    findContours(imgDil, contours, hiearchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); // find contours on binary image
    drawContours(img, contours, -1, Scalar(255, 0, 255), 2); //draw contours arround stars

    vector<elementDistance> distanceStars;

    for (int i = 0; i < contours.size(); i++) {  // Finded centers of stars
        Moments M = moments(contours[i]);
        Point center(M.m10 / M.m00, M.m01 / M.m00);
        centers.push_back(center);
    }

    centers.erase(unique(centers.begin(), centers.end()), centers.end()); //deleted duplicate values
    sort(centers.begin(), centers.end(), compareCoordinatesX);

    int clustered_counter = 0; 
    int unclustered_counter = 0;

    for (int i = 0; i < centers.size(); i++) { // Finded nearest stars index by index
        for (int j = 0; j < centers.size(); j++) {
            if (i != j) {
                double dist = distanceCalculate(centers[i].x, centers[i].y, centers[j].x, centers[j].y); //Calculate distance
                distanceStars.push_back({ i,j,dist });
            }
        }
        sort(distanceStars.begin(), distanceStars.end(), compareDistances); //Sorting distances

        for (int a = 0; a < 3; a++) //Print distance and indexes
        {
            cout << distanceStars[a].i << ", " << distanceStars[a].j
                << ", " << distanceStars[a].distance << "\n" << endl;
        }

        four_stars_after_calculate_distance.push_back(centers[distanceStars[0].i]); // Use top 4 centers of star
        four_stars_after_calculate_distance.push_back(centers[distanceStars[0].j]); // Find coordinates surrounding 4 stars
        four_stars_after_calculate_distance.push_back(centers[distanceStars[1].j]);
        four_stars_after_calculate_distance.push_back(centers[distanceStars[2].j]);

        sort(four_stars_after_calculate_distance.begin(), four_stars_after_calculate_distance.end(), compareCoordinatesX);

        auto x1 = min(min(four_stars_after_calculate_distance[0].x, four_stars_after_calculate_distance[1].x),
                    min(four_stars_after_calculate_distance[2].x, four_stars_after_calculate_distance[3].x));//top-left pt. is the leftmost of the 4 points
        auto x2 = max(max(four_stars_after_calculate_distance[0].x, four_stars_after_calculate_distance[1].x),
                    max(four_stars_after_calculate_distance[2].x, four_stars_after_calculate_distance[3].x));//bottom-right pt. is the rightmost of the 4 points
        auto y1 = min(min(four_stars_after_calculate_distance[0].y, four_stars_after_calculate_distance[1].y), 
                    min(four_stars_after_calculate_distance[2].y, four_stars_after_calculate_distance[3].y));//top-left pt. is the uppermost of the 4 points
        auto y2 = max(max(four_stars_after_calculate_distance[0].y, four_stars_after_calculate_distance[1].y),
                    max(four_stars_after_calculate_distance[2].y, four_stars_after_calculate_distance[3].y));//bottom-right pt. is the lowermost of the 4 points

        rectangle(img, Point(x1 - 5, y1 - 5), Point(x2 + 5, y2 + 5), Scalar(0, 22, 242)); // draw rect that surrounding 4 stars
        clustered_counter += 1;
            
        //Delete used coordinates of center 
        Point first_coordinate = centers[distanceStars[0].i];
        Point second_coordinate = centers[distanceStars[0].j];
        Point third_coordinate = centers[distanceStars[1].j];
        Point fourth_coordinate = centers[distanceStars[2].j];

            
        centers.erase(std::find(centers.begin(), centers.end(), first_coordinate));
        centers.erase(std::find(centers.begin(), centers.end(), second_coordinate));
        centers.erase(std::find(centers.begin(), centers.end(), third_coordinate));
        centers.erase(std::find(centers.begin(), centers.end(), fourth_coordinate));


        distanceStars.clear();
        four_stars_after_calculate_distance.clear();

        i = 0;
        if (centers.size() < 4) { // Draw circle on unclustered stars 
            vector<Point> remaining_elements;
            for (int z = 0; z < centers.size(); z++) {
                circle(img, centers[z], 5, Scalar(0, 255, 0), 2);
                unclustered_counter += 1;
            }
        }        
    }
    cout << "clustered star count:  " << clustered_counter << endl;
    rectangle(img, Point(45, 26), Point(500, 95), Scalar(0, 0, 255), FILLED);
    putText(img, "clustered star count: " + to_string(clustered_counter), Point(50, 50), 
        FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    putText(img, "unclustered star count: " + to_string(unclustered_counter), Point(50, 90), 
        FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    imshow("Clustered Stars", img);
    waitKey();
}


int main(int argc, char** argv)
{
    image = imread("Resources/StarMap.png"); // Load an image
    resize(image, image, Size(), 0.70, 0.70);
    cvtColor(image, image_gray, COLOR_BGR2GRAY); // Convert the image to Gray
    namedWindow(window_name, WINDOW_AUTOSIZE); // Create a window to display results

    createTrackbar(trackbar_value,
        window_name, &threshold_value,
        max_value, Threshold_and_Morphology_Operations); // Create a Trackbar to choose binary image
    createTrackbar(first_morp_bar,
        window_name, &morph_operator_1, 1, Threshold_and_Morphology_Operations);
    createTrackbar(kernel_1, window_name,
        &kernel_size_1, max_kernel_size,
        Threshold_and_Morphology_Operations);
    createTrackbar(second_morp_bar,
        window_name, &morph_operator_2, 1, Threshold_and_Morphology_Operations);
    createTrackbar(kernel_2, window_name,
        &kernel_size_2, max_kernel_size,
        Threshold_and_Morphology_Operations); // Trackbar can be turned off. Optimum values comes default.

    Threshold_and_Morphology_Operations(0, 0);
    waitKey();

    //imshow("Binary image which will use", binary_image);
    //waitKey();

    getCountoursAndClusterStars(binary_image, image);


    return 0;
}