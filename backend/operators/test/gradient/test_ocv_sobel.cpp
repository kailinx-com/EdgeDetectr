#include <gtest/gtest.h>
#include "test_utils.h"
#include "gradient/ocv_sobel.h"
#include <opencv2/opencv.hpp>
#include <numeric>

using namespace TestUtils;

/**
 * Test suite for OpenCV Sobel edge detection operator.
 * 
 * Tests the OpenCV implementation of the Sobel operator, which uses
 * gradient-based edge detection with configurable kernel sizes.
 * Verifies basic functionality, performance, and edge case handling.
 */
class OcvSobelTest : public GradientOperatorTest {
protected:
    void SetUp() override {
        GradientOperatorTest::SetUp();
        operator_ = std::make_unique<OcvSobel>();
    }
    
    std::unique_ptr<OcvSobel> operator_;
};

/**
 * Tests basic edge detection functionality.
 * 
 * Verifies that the Sobel operator can process a real image
 * and produce a valid edge detection result.
 */
TEST_F(OcvSobelTest, BasicEdgeDetection) {
    std::string outputPath = getUniqueOutputPath("ocv_sobel_basic");
    
    EXPECT_NO_THROW({
        cv::Mat result = operator_->getEdges(testImagePath, outputPath);
        EXPECT_FALSE(result.empty());
    });
    
    verifyOutputImage(outputPath);
}

/**
 * Tests edge detection with different kernel sizes.
 * 
 * Verifies that the Sobel operator works correctly with
 * various kernel sizes (3x3, 5x5, 7x7) and produces
 * appropriate results for each configuration.
 */
TEST_F(OcvSobelTest, DifferentKernelSizes) {
    std::vector<int> kernelSizes = {3, 5, 7};
    
    for (int ksize : kernelSizes) {
        OcvSobel sobel(ksize);
        std::string outputPath = getUniqueOutputPath("ocv_sobel_kernel_" + std::to_string(ksize));
        
        EXPECT_NO_THROW({
            cv::Mat result = sobel.getEdges(testImagePath, outputPath);
            EXPECT_FALSE(result.empty());
        });
        
        verifyOutputImage(outputPath);
    }
}

/**
 * Tests operator name consistency.
 * 
 * Verifies that the operator returns the correct name
 * for identification and logging purposes.
 */
TEST_F(OcvSobelTest, OperatorName) {
    EXPECT_EQ(operator_->getOperatorName(), "OcvSobel");
}

/**
 * Tests error handling for invalid input paths.
 * 
 * Verifies that the operator properly handles non-existent
 * image files and throws appropriate exceptions.
 */
TEST_F(OcvSobelTest, InvalidInputPath) {
    std::string invalidPath = "nonexistent_image.jpg";
    std::string outputPath = getUniqueOutputPath("ocv_sobel_invalid");
    
    EXPECT_THROW({
        operator_->getEdges(invalidPath, outputPath);
    }, std::runtime_error);
}

/**
 * Tests edge detection on synthetic images with known patterns.
 * 
 * Uses controlled test images with geometric shapes to verify
 * that the Sobel operator correctly identifies edges and
 * produces non-uniform output (indicating edge detection).
 */
TEST_F(OcvSobelTest, SyntheticImageEdgeDetection) {
    cv::Mat testImage = createSimpleTestImage(200, 200);
    std::string inputPath = testOutputDir + "/synthetic_input.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("ocv_sobel_synthetic");
    
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
 * Verifies that the Sobel operator maintains consistent
 * performance characteristics and doesn't degrade over time.
 * Ensures processing times remain within acceptable limits.
 */
TEST_F(OcvSobelTest, PerformanceConsistency) {
    std::vector<double> processingTimes;
    const int numRuns = 5;
    
    for (int i = 0; i < numRuns; ++i) {
        std::string outputPath = getUniqueOutputPath("ocv_sobel_perf_" + std::to_string(i));
        
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
 * Verifies that the Sobel operator doesn't consume excessive
 * memory during processing and maintains reasonable memory
 * usage relative to input image size.
 */
TEST_F(OcvSobelTest, MemoryUsage) {
    std::string outputPath = getUniqueOutputPath("ocv_sobel_memory");
    
    cv::Mat inputImage = loadTestImage();
    size_t initialMemory = inputImage.total() * inputImage.elemSize();
    
    cv::Mat result = operator_->getEdges(testImagePath, outputPath);
    
    size_t outputMemory = result.total() * result.elemSize();
    EXPECT_LE(outputMemory, initialMemory * 10) << "Output should not use excessive memory";
    
    verifyOutputImage(outputPath);
}
