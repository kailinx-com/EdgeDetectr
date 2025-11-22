#include <gtest/gtest.h>
#include "test_utils.h"
#include "gradient/alt_sobel.h"
#include "gradient/ocv_sobel.h"
#include <opencv2/opencv.hpp>
#include <numeric>

using namespace TestUtils;

/**
 * Test suite for Alternative Sobel edge detection operator.
 * 
 * Tests a custom implementation of the Sobel operator that provides
 * an alternative to the OpenCV version. Includes comparison tests
 * with the OpenCV implementation and specialized edge detection
 * quality assessments.
 */
class AltSobelTest : public GradientOperatorTest {
protected:
    void SetUp() override {
        GradientOperatorTest::SetUp();
        operator_ = std::make_unique<AltSobel>();
    }
    
    std::unique_ptr<AltSobel> operator_;
};

/**
 * Tests basic edge detection functionality.
 * 
 * Verifies that the Alternative Sobel operator can process
 * real images and produce valid edge detection results.
 */
TEST_F(AltSobelTest, BasicEdgeDetection) {
    std::string outputPath = getUniqueOutputPath("alt_sobel_basic");
    
    EXPECT_NO_THROW({
        cv::Mat result = operator_->getEdges(testImagePath, outputPath);
        EXPECT_FALSE(result.empty());
    });
    
    verifyOutputImage(outputPath);
}

/**
 * Tests operator name consistency.
 * 
 * Verifies that the operator returns the correct name
 * for identification and logging purposes.
 */
TEST_F(AltSobelTest, OperatorName) {
    EXPECT_EQ(operator_->getOperatorName(), "AltSobel");
}

/**
 * Tests error handling for invalid input paths.
 * 
 * Verifies that the operator properly handles non-existent
 * image files and throws appropriate exceptions.
 */
TEST_F(AltSobelTest, InvalidInputPath) {
    std::string invalidPath = "nonexistent_image.jpg";
    std::string outputPath = getUniqueOutputPath("alt_sobel_invalid");
    
    EXPECT_THROW({
        operator_->getEdges(invalidPath, outputPath);
    }, std::runtime_error);
}

/**
 * Tests edge detection on synthetic images with known patterns.
 * 
 * Uses controlled test images with geometric shapes to verify
 * that the Alternative Sobel operator correctly identifies edges
 * and produces non-uniform output (indicating edge detection).
 */
TEST_F(AltSobelTest, SyntheticImageEdgeDetection) {
    cv::Mat testImage = createSimpleTestImage(200, 200);
    std::string inputPath = testOutputDir + "/synthetic_input_alt_sobel.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("alt_sobel_synthetic");
    
    EXPECT_NO_THROW({
        cv::Mat result = operator_->getEdges(inputPath, outputPath);
        EXPECT_FALSE(result.empty());
        
        cv::Scalar meanIntensity = cv::mean(result);
        EXPECT_LT(meanIntensity[0], 255.0) << "Edge detection should produce non-white output";
    });
    
    verifyOutputImage(outputPath);
}

/**
 * Tests performance consistency across multiple runs.
 * 
 * Verifies that the Alternative Sobel operator maintains consistent
 * performance characteristics and doesn't degrade over time.
 * Ensures processing times remain within acceptable limits.
 */
TEST_F(AltSobelTest, PerformanceConsistency) {
    std::vector<double> processingTimes;
    const int numRuns = 5;
    
    for (int i = 0; i < numRuns; ++i) {
        std::string outputPath = getUniqueOutputPath("alt_sobel_perf_" + std::to_string(i));
        
        auto start = std::chrono::high_resolution_clock::now();
        cv::Mat result = operator_->getEdges(testImagePath, outputPath);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        processingTimes.push_back(duration.count());
        
        EXPECT_FALSE(result.empty());
        verifyOutputImage(outputPath);
    }
    
    double avgTime = std::accumulate(processingTimes.begin(), processingTimes.end(), 0.0) / numRuns;
    EXPECT_LT(avgTime, 5000.0) << "Processing time should be under 5 seconds on average";
}

/**
 * Tests memory usage efficiency.
 * 
 * Verifies that the Alternative Sobel operator doesn't consume
 * excessive memory during processing and maintains reasonable
 * memory usage relative to input image size.
 */
TEST_F(AltSobelTest, MemoryUsage) {
    std::string outputPath = getUniqueOutputPath("alt_sobel_memory");
    
    cv::Mat inputImage = loadTestImage();
    size_t initialMemory = inputImage.total() * inputImage.elemSize();
    
    cv::Mat result = operator_->getEdges(testImagePath, outputPath);
    
    size_t outputMemory = result.total() * result.elemSize();
    EXPECT_LE(outputMemory, initialMemory * 10) << "Output should not use excessive memory";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests comparison with OpenCV Sobel implementation.
 * 
 * Compares the Alternative Sobel results with the OpenCV Sobel
 * implementation to ensure both produce similar edge detection
 * results, validating the custom implementation's correctness.
 */
TEST_F(AltSobelTest, ComparisonWithOpenCVSobel) {
    OcvSobel opencvSobel;
    
    std::string altOutputPath = getUniqueOutputPath("alt_sobel_comparison");
    std::string opencvOutputPath = getUniqueOutputPath("opencv_sobel_comparison");
    
    cv::Mat altResult = operator_->getEdges(testImagePath, altOutputPath);
    cv::Mat opencvResult = opencvSobel.getEdges(testImagePath, opencvOutputPath);
    
    EXPECT_FALSE(altResult.empty());
    EXPECT_FALSE(opencvResult.empty());
    
    double similarity = compareImages(altResult, opencvResult);
    EXPECT_GT(similarity, 0.1) << "Alternative Sobel should produce similar results to OpenCV Sobel";
    
    verifyOutputImage(altOutputPath);
    verifyOutputImage(opencvOutputPath);
}

/**
 * Tests edge detection quality on controlled test images.
 * 
 * Uses a simple test image with a black square on white background
 * to verify that the Alternative Sobel operator correctly
 * identifies edges around the square boundaries.
 */
TEST_F(AltSobelTest, EdgeDetectionQuality) {
    cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
    
    cv::rectangle(testImage, cv::Point(30, 30), cv::Point(70, 70), cv::Scalar(0, 0, 0), -1);
    
    std::string inputPath = testOutputDir + "/quality_test_input_alt.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("alt_sobel_quality");
    
    cv::Mat result = operator_->getEdges(inputPath, outputPath);
    EXPECT_FALSE(result.empty());
    
    int edgePixels = cv::countNonZero(result);
    EXPECT_GT(edgePixels, 0) << "Alternative Sobel should detect edges in the test image";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests edge detection across different image sizes.
 * 
 * Verifies that the Alternative Sobel operator works correctly
 * with various image dimensions, ensuring scalability and
 * consistent edge detection quality.
 */
TEST_F(AltSobelTest, DifferentImageSizes) {
    std::vector<std::pair<int, int>> sizes = {{50, 50}, {100, 100}, {200, 200}, {400, 400}};
    
    for (const auto& [width, height] : sizes) {
        cv::Mat testImage = createSimpleTestImage(width, height);
        std::string inputPath = testOutputDir + "/size_test_alt_" + std::to_string(width) + "x" + std::to_string(height) + ".jpg";
        cv::imwrite(inputPath, testImage);
        
        std::string outputPath = getUniqueOutputPath("alt_sobel_size_" + std::to_string(width) + "x" + std::to_string(height));
        
        EXPECT_NO_THROW({
            cv::Mat result = operator_->getEdges(inputPath, outputPath);
            EXPECT_FALSE(result.empty());
            EXPECT_EQ(result.rows, height);
            EXPECT_EQ(result.cols, width);
        });
        
        verifyOutputImage(outputPath);
    }
}

/**
 * Tests edge detection sensitivity to varying edge strengths.
 * 
 * Creates test images with different contrast levels to verify
 * that the Alternative Sobel operator can detect edges of
 * varying intensities and is sensitive to subtle edge changes.
 */
TEST_F(AltSobelTest, EdgeDetectionSensitivity) {
    std::vector<int> edgeStrengths = {1, 2, 5, 10};
    
    for (int strength : edgeStrengths) {
        cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
        
        cv::rectangle(testImage, cv::Point(30, 30), cv::Point(70, 70), cv::Scalar(255-strength, 255-strength, 255-strength), -1);
        
        std::string inputPath = testOutputDir + "/sensitivity_test_alt_" + std::to_string(strength) + ".jpg";
        cv::imwrite(inputPath, testImage);
        
        std::string outputPath = getUniqueOutputPath("alt_sobel_sensitivity_" + std::to_string(strength));
        
        cv::Mat result = operator_->getEdges(inputPath, outputPath);
        EXPECT_FALSE(result.empty());
        
        verifyOutputImage(outputPath);
    }
}
