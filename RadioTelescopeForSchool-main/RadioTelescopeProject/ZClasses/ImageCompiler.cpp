#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "CImg.h"
#include <string>
#include <chrono>
#include <ctime> // trying a different time method... 
//ImageCompiler class by Gaby 

class ImageCompiler
{
public:
    std::vector<int> unorganizedImage;
    std::vector<std::vector<int>> organizedVectors;
    std::pair<int, int> detectionAreaSentFromGUI; //this is a pair object doesn't always have to be two ints but it's good for this case. 

    //Calls Detection Area
    void giveDetectionArea(const std::vector<int>& detectionArea)
    {
        //Detects if the detection area has 2 elements
        if (detectionArea.size() == 2)
        {
            //Uses make pair data type, this is used for heterogeneous objects and bound them together
            detectionAreaSentFromGUI = std::make_pair(detectionArea[0], detectionArea[1]);
            return;
        }
        //If detection area has the wrong format, outputs error message and empty vector. 
        else {
            std::cout << "Invalid detection area format. Expected '(x,y)'. " << std::endl;
            return; //Return null. 
        }
    }

    //Adds intensity to the Unorganized Image
    void addToImage(int intensity)
    {
        std::cout << "new Intesity of" <<intensity << "added" << std::endl;
        intensity = (intensity < 0) ? 0 : (intensity > 255) ? 255 : intensity;
        unorganizedImage.push_back(intensity);
    }

    void OrganizedVector()
    {


        std::cout << "vectors x and y " << detectionAreaSentFromGUI.first << detectionAreaSentFromGUI.second << "added" << std::endl; //testflag



        organizedVectors.clear();//clear it out... 

        for (int i = 0; i < detectionAreaSentFromGUI.second; i++)
        {
            std::vector<int> lineVector;
            for (int j = 0; j < detectionAreaSentFromGUI.first; j++)
            {
                // Calculate the index in the unorganizedImage vector
                int index = i * detectionAreaSentFromGUI.first + j;

                std::cout << "i: " << i << ", j: " << j << ", index: " << index << std::endl;

                // Check if the index is within the bounds of unorganizedImage
                if (index < unorganizedImage.size())
                {
                    lineVector.push_back(unorganizedImage[index]);
                }
                else
                {
                    // Handle the case where the index is out of bounds
                    std::cerr << "Index out of bounds: " << index << std::endl;
                    // You might want to decide how to handle this situation, e.g., push back a default value.
                    lineVector.push_back(0); // Add a default value
                }
            }
            organizedVectors.push_back(lineVector);
        }
        reverse(organizedVectors.begin(), organizedVectors.end());
        unorganizedImage.clear();
        // Make BMP picture or perform any other operations you need
        MakeABmpPicture();
    }

private:
    void MakeABmpPicture()
    {
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);

        
        std::string timestampStr = std::to_string(timestamp); //trying this little number out, saw it on github when trawling around for a library that does this for me from here on out... This is pretty low friction though. 

        // Create a unique filename with the timestamp
        std::string filename = "RadioAntennaOutput_" + timestampStr + ".bmp";

        // Width and height of the image
        const int width = detectionAreaSentFromGUI.first;
        const int height = detectionAreaSentFromGUI.second;

        // Create a CImg object with the specified width and height
        cimg_library::CImg<unsigned char> image(width, height, 1, 3);

        // Set pixel color based on intensity values, matrix operation.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int intensity = organizedVectors[y][x];

                // Set RGB values for each pixel
                image(x, y, 0) = intensity < 128 ? 2 * intensity : 0;         // Blue
                image(x, y, 1) = intensity < 128 ? intensity / 2 : 255 - intensity;  // Adjusted Green
                image(x, y, 2) = intensity >= 128 ? 2 * (intensity - 128) : 0;        // Red
            }
        }

        // Save the image to the unique BMP file
        image.save_bmp(filename.c_str());

        std::cout << "The BMP image is ready. Check " << filename << "." << std::endl;
    }
};