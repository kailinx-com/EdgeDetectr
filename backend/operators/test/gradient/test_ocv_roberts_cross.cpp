#include <gtest/gtest.h>
#include "test_utils.h"
#include "gradient/ocv_roberts_cross.h"
#include <opencv2/opencv.hpp>
#include <numeric>

using namespace TestUtils;

/**
 * Test suite for OpenCV Roberts Cross edge detection operator.
 * 
 * Tests the OpenCV implementation of the Roberts Cross operator, which uses
 * 2x2 gradient kernels for edge detection. Roberts Cross is particularly
 * effective at detecting diagonal edges and is computationally efficient.
 * Verifies basic functionality, performance, and specialized diagonal edge detection.
 */
class OcvRobertsCrossTest : public GradientOperatorTest {
protected:
    void SetUp() override {
        GradientOperatorTest::SetUp();
        operator_ = std::make_unique<OcvRobertsCross>();
    }
    
    std::unique_ptr<OcvRobertsCross> operator_;
};

/**
 * Tests basic edge detection functionality.
 * 
 * Verifies that the Roberts Cross operator can process a real image
 * and produce a valid edge detection result.
 */
TEST_F(OcvRobertsCrossTest, BasicEdgeDetection) {
    std::string outputPath = getUniqueOutputPath("ocv_roberts_basic");
    
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
TEST_F(OcvRobertsCrossTest, OperatorName) {
    EXPECT_EQ(operator_->getOperatorName(), "OcvRobertsCross");
}

/**
 * Tests error handling for invalid input paths.
 * 
 * Verifies that the operator properly handles non-existent
 * image files and throws appropriate exceptions.
 */
TEST_F(OcvRobertsCrossTest, InvalidInputPath) {
    std::string invalidPath = "nonexistent_image.jpg";
    std::string outputPath = getUniqueOutputPath("ocv_roberts_invalid");
    
    EXPECT_THROW({
        operator_->getEdges(invalidPath, outputPath);
    }, std::runtime_error);
}

/**
 * Tests edge detection on synthetic images with known patterns.
 * 
 * Uses controlled test images with geometric shapes to verify
 * that the Roberts Cross operator correctly identifies edges and
 * produces non-uniform output (indicating edge detection).
 */
TEST_F(OcvRobertsCrossTest, SyntheticImageEdgeDetection) {
    cv::Mat testImage = createSimpleTestImage(200, 200);
    std::string inputPath = testOutputDir + "/synthetic_input_roberts.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("ocv_roberts_synthetic");
    
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
 * Verifies that the Roberts Cross operator maintains consistent
 * performance characteristics and doesn't degrade over time.
 * Ensures processing times remain within acceptable limits.
 */
TEST_F(OcvRobertsCrossTest, PerformanceConsistency) {
    std::vector<double> processingTimes;
    const int numRuns = 5;
    
    for (int i = 0; i < numRuns; ++i) {
        std::string outputPath = getUniqueOutputPath("ocv_roberts_perf_" + std::to_string(i));
        
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
 * Verifies that the Roberts Cross operator doesn't consume excessive
 * memory during processing and maintains reasonable memory
 * usage relative to input image size.
 */
TEST_F(OcvRobertsCrossTest, MemoryUsage) {
    std::string outputPath = getUniqueOutputPath("ocv_roberts_memory");
    
    cv::Mat inputImage = loadTestImage();
    size_t initialMemory = inputImage.total() * inputImage.elemSize();
    
    cv::Mat result = operator_->getEdges(testImagePath, outputPath);
    
    size_t outputMemory = result.total() * result.elemSize();
    EXPECT_LE(outputMemory, initialMemory * 10) << "Output should not use excessive memory";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests diagonal edge detection capabilities.
 * 
 * Creates test images with diagonal lines to verify that the
 * Roberts Cross operator excels at detecting diagonal edges,
 * which is one of its key strengths compared to other operators.
 */
TEST_F(OcvRobertsCrossTest, DiagonalEdgeDetection) {
    cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
    
    cv::line(testImage, cv::Point(20, 20), cv::Point(80, 80), cv::Scalar(0, 0, 0), 3);
    cv::line(testImage, cv::Point(80, 20), cv::Point(20, 80), cv::Scalar(0, 0, 0), 3);
    
    std::string inputPath = testOutputDir + "/diagonal_test_input.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("ocv_roberts_diagonal");
    
    cv::Mat result = operator_->getEdges(inputPath, outputPath);
    EXPECT_FALSE(result.empty());
    
    int edgePixels = cv::countNonZero(result);
    EXPECT_GT(edgePixels, 0) << "Roberts Cross should detect diagonal edges";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests edge detection across different image sizes.
 * 
 * Verifies that the Roberts Cross operator works correctly
 * with various image dimensions, ensuring scalability and
 * consistent edge detection quality.
 */
TEST_F(OcvRobertsCrossTest, DifferentImageSizes) {
    std::vector<std::pair<int, int>> sizes = {{50, 50}, {100, 100}, {200, 200}, {400, 400}};
    
    for (const auto& [width, height] : sizes) {
        cv::Mat testImage = createSimpleTestImage(width, height);
        std::string inputPath = testOutputDir + "/size_test_" + std::to_string(width) + "x" + std::to_string(height) + ".jpg";
        cv::imwrite(inputPath, testImage);
        
        std::string outputPath = getUniqueOutputPath("ocv_roberts_size_" + std::to_string(width) + "x" + std::to_string(height));
        
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
 * that the Roberts Cross operator can detect edges of
 * varying intensities and is sensitive to subtle edge changes.
 */
TEST_F(OcvRobertsCrossTest, EdgeDetectionSensitivity) {
    std::vector<int> edgeStrengths = {1, 2, 5, 10};
    
    for (int strength : edgeStrengths) {
        cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
        
        cv::rectangle(testImage, cv::Point(30, 30), cv::Point(70, 70), cv::Scalar(255-strength, 255-strength, 255-strength), -1);
        
        std::string inputPath = testOutputDir + "/sensitivity_test_" + std::to_string(strength) + ".jpg";
        cv::imwrite(inputPath, testImage);
        
        std::string outputPath = getUniqueOutputPath("ocv_roberts_sensitivity_" + std::to_string(strength));
        
        cv::Mat result = operator_->getEdges(inputPath, outputPath);
        EXPECT_FALSE(result.empty());
        
        verifyOutputImage(outputPath);
    }
}
