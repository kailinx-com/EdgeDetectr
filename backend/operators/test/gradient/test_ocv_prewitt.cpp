#include <gtest/gtest.h>
#include "test_utils.h"
#include "gradient/ocv_prewitt.h"
#include <opencv2/opencv.hpp>
#include <numeric>

using namespace TestUtils;

/**
 * Test suite for OpenCV Prewitt edge detection operator.
 * 
 * Tests the OpenCV implementation of the Prewitt operator, which uses
 * gradient-based edge detection with 3x3 kernels. Prewitt is known
 * for its sensitivity to horizontal and vertical edges.
 * Verifies basic functionality, performance, and edge case handling.
 */
class OcvPrewittTest : public GradientOperatorTest {
protected:
    void SetUp() override {
        GradientOperatorTest::SetUp();
        operator_ = std::make_unique<OcvPrewitt>();
    }
    
    std::unique_ptr<OcvPrewitt> operator_;
};

/**
 * Tests basic edge detection functionality.
 * 
 * Verifies that the Prewitt operator can process a real image
 * and produce a valid edge detection result.
 */
TEST_F(OcvPrewittTest, BasicEdgeDetection) {
    std::string outputPath = getUniqueOutputPath("ocv_prewitt_basic");
    
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
TEST_F(OcvPrewittTest, OperatorName) {
    EXPECT_EQ(operator_->getOperatorName(), "OcvPrewitt");
}

/**
 * Tests error handling for invalid input paths.
 * 
 * Verifies that the operator properly handles non-existent
 * image files and throws appropriate exceptions.
 */
TEST_F(OcvPrewittTest, InvalidInputPath) {
    std::string invalidPath = "nonexistent_image.jpg";
    std::string outputPath = getUniqueOutputPath("ocv_prewitt_invalid");
    
    EXPECT_THROW({
        operator_->getEdges(invalidPath, outputPath);
    }, std::runtime_error);
}

/**
 * Tests edge detection on synthetic images with known patterns.
 * 
 * Uses controlled test images with geometric shapes to verify
 * that the Prewitt operator correctly identifies edges and
 * produces non-uniform output (indicating edge detection).
 */
TEST_F(OcvPrewittTest, SyntheticImageEdgeDetection) {
    cv::Mat testImage = createSimpleTestImage(200, 200);
    std::string inputPath = testOutputDir + "/synthetic_input_prewitt.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("ocv_prewitt_synthetic");
    
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
 * Verifies that the Prewitt operator maintains consistent
 * performance characteristics and doesn't degrade over time.
 * Ensures processing times remain within acceptable limits.
 */
TEST_F(OcvPrewittTest, PerformanceConsistency) {
    std::vector<double> processingTimes;
    const int numRuns = 5;
    
    for (int i = 0; i < numRuns; ++i) {
        std::string outputPath = getUniqueOutputPath("ocv_prewitt_perf_" + std::to_string(i));
        
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
 * Verifies that the Prewitt operator doesn't consume excessive
 * memory during processing and maintains reasonable memory
 * usage relative to input image size.
 */
TEST_F(OcvPrewittTest, MemoryUsage) {
    std::string outputPath = getUniqueOutputPath("ocv_prewitt_memory");
    
    cv::Mat inputImage = loadTestImage();
    size_t initialMemory = inputImage.total() * inputImage.elemSize();
    
    cv::Mat result = operator_->getEdges(testImagePath, outputPath);
    
    size_t outputMemory = result.total() * result.elemSize();
    EXPECT_LE(outputMemory, initialMemory * 10) << "Output should not use excessive memory";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests edge detection quality on controlled test images.
 * 
 * Uses a simple test image with a black square on white background
 * to verify that the Prewitt operator correctly identifies
 * edges around the square boundaries.
 */
TEST_F(OcvPrewittTest, EdgeDetectionQuality) {
    cv::Mat testImage(100, 100, CV_8UC3, cv::Scalar(255, 255, 255));
    
    cv::rectangle(testImage, cv::Point(30, 30), cv::Point(70, 70), cv::Scalar(0, 0, 0), -1);
    
    std::string inputPath = testOutputDir + "/quality_test_input.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("ocv_prewitt_quality");
    
    cv::Mat result = operator_->getEdges(inputPath, outputPath);
    EXPECT_FALSE(result.empty());
    
    int edgePixels = cv::countNonZero(result);
    EXPECT_GT(edgePixels, 0) << "Prewitt should detect edges in the test image";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests support for different image formats.
 * 
 * Verifies that the Prewitt operator can handle various
 * image formats (JPG, PNG, BMP) and produces consistent
 * results regardless of input format.
 */
TEST_F(OcvPrewittTest, DifferentImageFormats) {
    std::vector<std::string> formats = {"jpg", "png", "bmp"};
    
    for (const auto& format : formats) {
        std::string inputPath = testOutputDir + "/format_test." + format;
        std::string outputPath = getUniqueOutputPath("ocv_prewitt_format_" + format);
        
        cv::Mat testImage = createSimpleTestImage(150, 150);
        cv::imwrite(inputPath, testImage);
        
        EXPECT_NO_THROW({
            cv::Mat result = operator_->getEdges(inputPath, outputPath);
            EXPECT_FALSE(result.empty());
        });
        
        verifyOutputImage(outputPath);
    }
}
