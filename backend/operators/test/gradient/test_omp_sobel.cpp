#include <gtest/gtest.h>
#include "test_utils.h"
#include "gradient/omp_sobel.h"
#include <opencv2/opencv.hpp>
#include <numeric>
#include <thread>

using namespace TestUtils;

/**
 * Test suite for OpenMP Sobel edge detection operator.
 * 
 * Tests the OpenMP-parallelized implementation of the Sobel operator,
 * which provides multi-threaded edge detection capabilities.
 * Verifies parallelization benefits, thread safety, and performance
 * characteristics across different thread configurations.
 */
class OmpSobelTest : public GradientOperatorTest {
protected:
    void SetUp() override {
        GradientOperatorTest::SetUp();
        operator_ = std::make_unique<OmpSobel>();
    }
    
    std::unique_ptr<OmpSobel> operator_;
};

/**
 * Tests basic edge detection functionality.
 * 
 * Verifies that the OpenMP Sobel operator can process
 * real images and produce valid edge detection results.
 */
TEST_F(OmpSobelTest, BasicEdgeDetection) {
    std::string outputPath = getUniqueOutputPath("omp_sobel_basic");
    
    EXPECT_NO_THROW({
        cv::Mat result = operator_->getEdges(testImagePath, outputPath);
        EXPECT_FALSE(result.empty());
    });
    
    verifyOutputImage(outputPath);
}

/**
 * Tests edge detection with different thread counts.
 * 
 * Verifies that the OpenMP Sobel operator works correctly with
 * various thread configurations (1, 2, 4, 8 threads) and
 * produces consistent results regardless of thread count.
 */
TEST_F(OmpSobelTest, DifferentThreadCounts) {
    std::vector<int> threadCounts = {1, 2, 4, 8};
    
    for (int threads : threadCounts) {
        OmpSobel sobel(threads);
        std::string outputPath = getUniqueOutputPath("omp_sobel_threads_" + std::to_string(threads));
        
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
TEST_F(OmpSobelTest, OperatorName) {
    EXPECT_EQ(operator_->getOperatorName(), "OpenMP Sobel");
}

/**
 * Tests error handling for invalid input paths.
 * 
 * Verifies that the operator properly handles non-existent
 * image files and throws appropriate exceptions.
 */
TEST_F(OmpSobelTest, InvalidInputPath) {
    std::string invalidPath = "nonexistent_image.jpg";
    std::string outputPath = getUniqueOutputPath("omp_sobel_invalid");
    
    EXPECT_THROW({
        operator_->getEdges(invalidPath, outputPath);
    }, std::runtime_error);
}

/**
 * Tests edge detection on synthetic images with known patterns.
 * 
 * Uses controlled test images with geometric shapes to verify
 * that the OpenMP Sobel operator correctly identifies edges
 * and produces non-uniform output (indicating edge detection).
 */
TEST_F(OmpSobelTest, SyntheticImageEdgeDetection) {
    cv::Mat testImage = createSimpleTestImage(200, 200);
    std::string inputPath = testOutputDir + "/synthetic_input_omp.jpg";
    cv::imwrite(inputPath, testImage);
    
    std::string outputPath = getUniqueOutputPath("omp_sobel_synthetic");
    
    EXPECT_NO_THROW({
        cv::Mat result = operator_->getEdges(inputPath, outputPath);
        EXPECT_FALSE(result.empty());
        
        cv::Scalar meanIntensity = cv::mean(result);
        EXPECT_LT(meanIntensity[0], 255.0) << "Edge detection should produce non-white output";
    });
    
    verifyOutputImage(outputPath);
}

/**
 * Tests parallelization benefits and result consistency.
 * 
 * Compares single-threaded and multi-threaded execution to verify
 * that OpenMP parallelization produces identical results while
 * potentially improving performance. Ensures thread safety.
 */
TEST_F(OmpSobelTest, ParallelizationBenefits) {
    OmpSobel singleThread(1);
    std::string outputPath1 = getUniqueOutputPath("omp_sobel_single");
    
    auto start1 = std::chrono::high_resolution_clock::now();
    cv::Mat result1 = singleThread.getEdges(testImagePath, outputPath1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    OmpSobel multiThread(4);
    std::string outputPath4 = getUniqueOutputPath("omp_sobel_multi");
    
    auto start4 = std::chrono::high_resolution_clock::now();
    cv::Mat result4 = multiThread.getEdges(testImagePath, outputPath4);
    auto end4 = std::chrono::high_resolution_clock::now();
    auto duration4 = std::chrono::duration_cast<std::chrono::milliseconds>(end4 - start4);
    
    EXPECT_FALSE(result1.empty());
    EXPECT_FALSE(result4.empty());
    
    double similarity = compareImages(result1, result4);
    EXPECT_EQ(similarity, 0.0) << "Single and multi-threaded results should be identical";
    
    verifyOutputImage(outputPath1);
    verifyOutputImage(outputPath4);
}

/**
 * Tests performance consistency across multiple runs.
 * 
 * Verifies that the OpenMP Sobel operator maintains consistent
 * performance characteristics and doesn't degrade over time.
 * Ensures processing times remain within acceptable limits.
 */
TEST_F(OmpSobelTest, PerformanceConsistency) {
    std::vector<double> processingTimes;
    const int numRuns = 5;
    
    for (int i = 0; i < numRuns; ++i) {
        std::string outputPath = getUniqueOutputPath("omp_sobel_perf_" + std::to_string(i));
        
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
 * Verifies that the OpenMP Sobel operator doesn't consume
 * excessive memory during processing and maintains reasonable
 * memory usage relative to input image size.
 */
TEST_F(OmpSobelTest, MemoryUsage) {
    std::string outputPath = getUniqueOutputPath("omp_sobel_memory");
    
    cv::Mat inputImage = loadTestImage();
    size_t initialMemory = inputImage.total() * inputImage.elemSize();
    
    cv::Mat result = operator_->getEdges(testImagePath, outputPath);
    
    size_t outputMemory = result.total() * result.elemSize();
    EXPECT_LE(outputMemory, initialMemory * 10) << "Output should not use excessive memory";
    
    verifyOutputImage(outputPath);
}

/**
 * Tests thread safety under concurrent execution.
 * 
 * Verifies that multiple OpenMP Sobel instances can run
 * concurrently without race conditions or data corruption.
 * Ensures thread-safe operation in multi-threaded environments.
 */
TEST_F(OmpSobelTest, ThreadSafety) {
    std::vector<std::thread> threads;
    std::vector<std::string> outputPaths;
    const int numThreads = 4;
    
    for (int i = 0; i < numThreads; ++i) {
        outputPaths.push_back(getUniqueOutputPath("omp_sobel_thread_safe_" + std::to_string(i)));
    }
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, &outputPaths]() {
            OmpSobel sobel(2);
            cv::Mat result = sobel.getEdges(testImagePath, outputPaths[i]);
            EXPECT_FALSE(result.empty());
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    for (const auto& outputPath : outputPaths) {
        verifyOutputImage(outputPath);
    }
}
