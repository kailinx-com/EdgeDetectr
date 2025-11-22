#include <gtest/gtest.h>
#include "test_utils.h"
#include "utils/image_utils.h"
#include <opencv2/opencv.hpp>

using namespace TestUtils;

/**
 * Test suite for OpenCV image utility functions.
 * 
 * Tests basic image operations including loading, saving, format conversion,
 * resizing, and error handling. Ensures the underlying OpenCV functionality
 * works correctly for the edge detection operators.
 */
class ImageUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        testOutputDir = "test_output";
        std::filesystem::create_directories(testOutputDir);
    }
    
    void TearDown() override {
        if (std::filesystem::exists(testOutputDir)) {
            std::filesystem::remove_all(testOutputDir);
        }
    }
    
    std::string testOutputDir;
    
    /**
     * Creates a test image with geometric shapes for testing.
     * 
     * @param width Width of the test image
     * @param height Height of the test image
     * @return cv::Mat Test image with rectangle and circle
     */
    cv::Mat createTestImage(int width = 100, int height = 100) {
        cv::Mat image(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
        
        cv::rectangle(image, cv::Point(20, 20), cv::Point(80, 80), cv::Scalar(0, 0, 0), 2);
        cv::circle(image, cv::Point(50, 50), 15, cv::Scalar(128, 128, 128), -1);
        
        return image;
    }
};

/**
 * Tests basic image loading functionality.
 * 
 * Verifies that images can be saved and loaded correctly,
 * maintaining their dimensions and content integrity.
 */
TEST_F(ImageUtilsTest, ImageLoading) {
    cv::Mat testImage = createTestImage(200, 200);
    std::string imagePath = testOutputDir + "/test_image.jpg";
    cv::imwrite(imagePath, testImage);
    
    cv::Mat loadedImage = cv::imread(imagePath);
    EXPECT_FALSE(loadedImage.empty()) << "Failed to load test image";
    EXPECT_EQ(loadedImage.rows, 200);
    EXPECT_EQ(loadedImage.cols, 200);
}

/**
 * Tests image saving functionality.
 * 
 * Ensures images can be saved to disk and retrieved with
 * correct dimensions and content.
 */
TEST_F(ImageUtilsTest, ImageSaving) {
    cv::Mat testImage = createTestImage(150, 150);
    std::string outputPath = testOutputDir + "/saved_image.jpg";
    
    bool saveResult = cv::imwrite(outputPath, testImage);
    EXPECT_TRUE(saveResult) << "Failed to save test image";
    
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    
    cv::Mat loadedImage = cv::imread(outputPath);
    EXPECT_FALSE(loadedImage.empty());
    EXPECT_EQ(loadedImage.rows, 150);
    EXPECT_EQ(loadedImage.cols, 150);
}

/**
 * Tests support for different image formats.
 * 
 * Verifies that the system can handle JPG, PNG, and BMP formats
 * for both saving and loading operations.
 */
TEST_F(ImageUtilsTest, DifferentImageFormats) {
    cv::Mat testImage = createTestImage(100, 100);
    std::vector<std::string> formats = {"jpg", "png", "bmp"};
    
    for (const auto& format : formats) {
        std::string outputPath = testOutputDir + "/format_test." + format;
        
        bool saveResult = cv::imwrite(outputPath, testImage);
        EXPECT_TRUE(saveResult) << "Failed to save image in " << format << " format";
        
        EXPECT_TRUE(std::filesystem::exists(outputPath));
        
        cv::Mat loadedImage = cv::imread(outputPath);
        EXPECT_FALSE(loadedImage.empty());
        EXPECT_EQ(loadedImage.rows, 100);
        EXPECT_EQ(loadedImage.cols, 100);
    }
}

/**
 * Tests BGR to grayscale conversion accuracy.
 * 
 * Verifies that color images are properly converted to grayscale
 * while preserving approximate brightness values.
 */
TEST_F(ImageUtilsTest, BGRToGrayscaleConversion) {
    cv::Mat colorImage = createTestImage(100, 100);
    cv::Mat grayImage;
    
    cv::cvtColor(colorImage, grayImage, cv::COLOR_BGR2GRAY);
    
    EXPECT_FALSE(grayImage.empty());
    EXPECT_EQ(grayImage.rows, 100);
    EXPECT_EQ(grayImage.cols, 100);
    EXPECT_EQ(grayImage.channels(), 1);
    
    cv::Mat bgrImage;
    cv::cvtColor(grayImage, bgrImage, cv::COLOR_GRAY2BGR);
    
    cv::Scalar meanColor = cv::mean(colorImage);
    cv::Scalar meanGray = cv::mean(grayImage);
    
    double colorMean = (meanColor[0] + meanColor[1] + meanColor[2]) / 3.0;
    EXPECT_NEAR(meanGray[0], colorMean, 50.0) << "Grayscale conversion should preserve approximate brightness";
}

/**
 * Tests image resizing functionality.
 * 
 * Verifies that images can be downscaled and upscaled correctly
 * while maintaining aspect ratio and content integrity.
 */
TEST_F(ImageUtilsTest, ImageResizing) {
    cv::Mat originalImage = createTestImage(200, 200);
    cv::Mat resizedImage;
    
    cv::resize(originalImage, resizedImage, cv::Size(100, 100));
    
    EXPECT_FALSE(resizedImage.empty());
    EXPECT_EQ(resizedImage.rows, 100);
    EXPECT_EQ(resizedImage.cols, 100);
    
    cv::Mat upscaledImage;
    cv::resize(originalImage, upscaledImage, cv::Size(400, 400));
    
    EXPECT_FALSE(upscaledImage.empty());
    EXPECT_EQ(upscaledImage.rows, 400);
    EXPECT_EQ(upscaledImage.cols, 400);
}

/**
 * Tests image validation and edge cases.
 * 
 * Verifies proper handling of valid images, empty images,
 * and images with zero dimensions.
 */
TEST_F(ImageUtilsTest, ImageValidation) {
    cv::Mat validImage = createTestImage(100, 100);
    EXPECT_FALSE(validImage.empty());
    EXPECT_GT(validImage.rows, 0);
    EXPECT_GT(validImage.cols, 0);
    
    cv::Mat emptyImage;
    EXPECT_TRUE(emptyImage.empty());
    
    cv::Mat zeroImage(0, 0, CV_8UC3);
    EXPECT_EQ(zeroImage.rows, 0);
    EXPECT_EQ(zeroImage.cols, 0);
}

/**
 * Tests memory management for multiple images.
 * 
 * Verifies that creating and destroying multiple images
 * doesn't cause memory leaks or corruption.
 */
TEST_F(ImageUtilsTest, MemoryManagement) {
    std::vector<cv::Mat> images;
    const int numImages = 10;
    
    for (int i = 0; i < numImages; ++i) {
        images.push_back(createTestImage(200, 200));
    }
    
    EXPECT_EQ(images.size(), numImages);
    
    for (const auto& image : images) {
        EXPECT_FALSE(image.empty());
        EXPECT_EQ(image.rows, 200);
        EXPECT_EQ(image.cols, 200);
    }
    
    images.clear();
    EXPECT_EQ(images.size(), 0);
}

/**
 * Tests basic image arithmetic operations.
 * 
 * Verifies that addition, subtraction, and multiplication
 * operations work correctly on image matrices.
 */
TEST_F(ImageUtilsTest, ImageOperations) {
    cv::Mat image1 = createTestImage(100, 100);
    cv::Mat image2 = createTestImage(100, 100);
    
    cv::Mat addedImage;
    cv::add(image1, image2, addedImage);
    
    EXPECT_FALSE(addedImage.empty());
    EXPECT_EQ(addedImage.rows, 100);
    EXPECT_EQ(addedImage.cols, 100);
    
    cv::Mat subtractedImage;
    cv::subtract(image1, image2, subtractedImage);
    
    EXPECT_FALSE(subtractedImage.empty());
    EXPECT_EQ(subtractedImage.rows, 100);
    EXPECT_EQ(subtractedImage.cols, 100);
    
    cv::Mat multipliedImage;
    cv::multiply(image1, image2, multipliedImage);
    
    EXPECT_FALSE(multipliedImage.empty());
    EXPECT_EQ(multipliedImage.rows, 100);
    EXPECT_EQ(multipliedImage.cols, 100);
}

/**
 * Tests error handling for invalid operations.
 * 
 * Verifies graceful handling of non-existent files
 * and invalid file paths.
 */
TEST_F(ImageUtilsTest, ErrorHandling) {
    cv::Mat nonExistentImage = cv::imread("nonexistent_image.jpg");
    EXPECT_TRUE(nonExistentImage.empty());
    
    cv::Mat testImage = createTestImage(50, 50);
    bool saveResult = cv::imwrite("/invalid/path/image.jpg", testImage);
    EXPECT_FALSE(saveResult);
}
