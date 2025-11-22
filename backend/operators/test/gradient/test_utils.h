#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>

namespace TestUtils {

/**
 * Base test fixture for gradient operator testing.
 * 
 * Provides common setup/teardown functionality and utility methods
 * for testing edge detection operators. Handles test image loading,
 * output directory management, and result validation.
 */
class GradientOperatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        testOutputDir = "test_output";
        std::filesystem::create_directories(testOutputDir);
        
        testImagePath = std::filesystem::current_path().parent_path().string() + "/test/gradient/datasets/image.jpg";
        
        ASSERT_TRUE(std::filesystem::exists(testImagePath)) 
            << "Test image not found: " << testImagePath;
    }
    
    void TearDown() override {
        if (std::filesystem::exists(testOutputDir)) {
            std::filesystem::remove_all(testOutputDir);
        }
    }
    
    std::string testOutputDir;
    std::string testImagePath;
    
    /**
     * Loads the standard test image for edge detection testing.
     * 
     * @return cv::Mat The loaded test image
     * @throws std::runtime_error if image cannot be loaded
     */
    cv::Mat loadTestImage() {
        cv::Mat image = cv::imread(testImagePath);
        EXPECT_FALSE(image.empty()) << "Failed to load test image: " << testImagePath;
        return image;
    }
    
    /**
     * Generates a unique output filename for test results.
     * 
     * Uses timestamp to ensure uniqueness across test runs.
     * 
     * @param operatorName Name of the operator being tested
     * @return std::string Unique output path
     */
    std::string getUniqueOutputPath(const std::string& operatorName) {
        return testOutputDir + "/" + operatorName + "_" + 
               std::to_string(std::time(nullptr)) + ".jpg";
    }
    
    /**
     * Validates that an output image was created and is properly formatted.
     * 
     * Checks file existence, loads the image, and verifies basic properties.
     * 
     * @param outputPath Path to the output image to validate
     */
    void verifyOutputImage(const std::string& outputPath) {
        EXPECT_TRUE(std::filesystem::exists(outputPath)) 
            << "Output image not created: " << outputPath;
        
        cv::Mat outputImage = cv::imread(outputPath);
        EXPECT_FALSE(outputImage.empty()) 
            << "Output image is empty: " << outputPath;
        
        EXPECT_GT(outputImage.rows, 0) << "Output image has no rows";
        EXPECT_GT(outputImage.cols, 0) << "Output image has no columns";
    }
    
    /**
     * Compares two images and returns their similarity score.
     * 
     * Calculates the mean absolute difference between images.
     * Lower values indicate more similar images.
     * 
     * @param img1 First image for comparison
     * @param img2 Second image for comparison
     * @return double Similarity score (0 = identical, higher = more different)
     */
    double compareImages(const cv::Mat& img1, const cv::Mat& img2) {
        if (img1.size() != img2.size()) {
            return 0.0;
        }
        
        cv::Mat diff;
        cv::absdiff(img1, img2, diff);
        
        cv::Scalar meanDiff = cv::mean(diff);
        
        if (img1.channels() == 1) {
            return meanDiff[0];
        } else {
            return meanDiff[0] + meanDiff[1] + meanDiff[2];
        }
    }
    
    /**
     * Creates a synthetic test image with known edge patterns.
     * 
     * Generates a white background with black rectangle and gray circle
     * to test edge detection algorithms on controlled input.
     * 
     * @param width Width of the test image
     * @param height Height of the test image
     * @return cv::Mat Synthetic test image
     */
    cv::Mat createSimpleTestImage(int width = 100, int height = 100) {
        cv::Mat image(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
        
        cv::rectangle(image, cv::Point(20, 20), cv::Point(80, 80), cv::Scalar(0, 0, 0), 2);
        cv::circle(image, cv::Point(50, 50), 15, cv::Scalar(128, 128, 128), -1);
        
        return image;
    }
};

} // namespace TestUtils

#endif // TEST_UTILS_H
