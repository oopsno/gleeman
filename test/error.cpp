#include <catch.hpp>
#include <nvml.h>
#include <cuda_runtime.h>

#include "gleeman/exception.hpp"
#include "gleeman/error.hpp"

TEST_CASE("gleeman::error_traits") {
  auto runtime_success = gleeman::error_traits<cudaError_t>::success;
  auto nvml_success = gleeman::error_traits<nvmlReturn_t>::success;

  REQUIRE(runtime_success == cudaSuccess);
  REQUIRE(nvml_success == NVML_SUCCESS);
}

TEST_CASE("gleeman::UniformedError") {
  gleeman::UniformedError error;

  SECTION("default constructor") {
    REQUIRE(error == CUDA_SUCCESS);
    REQUIRE(error == cudaSuccess);
    REQUIRE(error == NVML_SUCCESS);
  }

  SECTION("operator=") {
    CHECK_THROWS_AS(error.handle(CUDA_ERROR_OUT_OF_MEMORY), std::runtime_error);
    REQUIRE(error == CUDA_ERROR_OUT_OF_MEMORY);

    CHECK_THROWS_AS(error.handle(cudaErrorMemoryAllocation), std::runtime_error);
    REQUIRE(error == cudaErrorMemoryAllocation);

#ifdef USE_NVML
    CHECK_THROWS_AS(error.handle(NVML_ERROR_TIMEOUT), std::runtime_error);
#else
    CHECK_THROWS_AS(error.handle(NVML_ERROR_TIMEOUT), gleeman::NoNVMLError);
#endif
    REQUIRE(error == NVML_ERROR_TIMEOUT);
  }
}