// #define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>


#include "util/pch.h"
#include "util/math/math.h"
#include "util/math/random.h" 




TEST_CASE("Random number generation", "[random]") {

    SECTION("Random class construction") {
        AT::util::random rng1;
        AT::util::random rng2(12345); // With specific seed
        
        // Both should be constructible
        REQUIRE_NOTHROW(AT::util::random());
        REQUIRE_NOTHROW(AT::util::random(12345));
    }
    
    SECTION("Integer random generation") {
        AT::util::random rng(42); // Fixed seed for reproducible tests
        
        // Test integer generation within range
        for (int i = 0; i < 100; ++i) {
            int value = rng.get<int>(0, 10);
            REQUIRE(value >= 0);
            REQUIRE(value <= 10);
        }
        
        // Test negative ranges
        int negative_val = rng.get<int>(-5, 5);
        REQUIRE(negative_val >= -5);
        REQUIRE(negative_val <= 5);
    }
    
    SECTION("Floating point random generation") {
        AT::util::random rng(42);
        
        // Test f32 generation within range
        for (int i = 0; i < 100; ++i) {
            f32 value = rng.get<f32>(0.0f, 1.0f);
            REQUIRE(value >= 0.0f);
            REQUIRE(value <= 1.0f);
        }
        
        // Test negative ranges
        f32 negative_val = rng.get<f32>(-1.0f, 1.0f);
        REQUIRE(negative_val >= -1.0f);
        REQUIRE(negative_val <= 1.0f);
    }
    
    SECTION("Vector generation") {
        AT::util::random rng(42);
        
        // Test vec3 generation
        for (int i = 0; i < 10; ++i) {
            glm::vec3 vec = rng.get_vec3(-1.0f, 1.0f);
            REQUIRE(vec.x >= -1.0f);
            REQUIRE(vec.x <= 1.0f);
            REQUIRE(vec.y >= -1.0f);
            REQUIRE(vec.y <= 1.0f);
            REQUIRE(vec.z >= -1.0f);
            REQUIRE(vec.z <= 1.0f);
        }
        
        // Test with different ranges
        glm::vec3 vec = rng.get_vec3(5.0f, 10.0f);
        REQUIRE(vec.x >= 5.0f);
        REQUIRE(vec.x <= 10.0f);
    }
    
    SECTION("Percentage test") {
        AT::util::random rng(42);
        
        // Test with 0% probability
        REQUIRE_FALSE(rng.get_percent(0.0f));
        
        // Test with 100% probability
        REQUIRE(rng.get_percent(1.0f));
        
        // Count successes for 50% probability
        int successes = 0;
        const int trials = 1000;
        for (int i = 0; i < trials; ++i) {
            if (rng.get_percent(0.5f)) {
                successes++;
            }
        }
        
        // Should be roughly 50% (allow for some randomness)
        REQUIRE(successes > 400);
        REQUIRE(successes < 600);
    }
    
    SECTION("String generation") {
        AT::util::random rng(42);
        
        for (int length : {0, 1, 5, 10, 100}) {                     // Test different lengths
            std::string str = rng.get_string(length);
            REQUIRE(str.length() == (size_t)length);
            
            // Check that all characters are from the expected set
            const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            for (char c : str)
                REQUIRE(charset.find(c) != std::string::npos);
        }
        
        // Test uniqueness (not guaranteed but should be likely with different seeds)
        AT::util::random rng1(1);
        AT::util::random rng2(2);
        
        std::string str1 = rng1.get_string(10);
        std::string str2 = rng2.get_string(10);
        
        // They should probably be different with different seeds
        // (Note: This could theoretically fail but is extremely unlikely)
        REQUIRE(str1 != str2);
    }
    
    SECTION("Deterministic with same seed") {
        // Two RNGs with the same seed should produce the same sequence
        AT::util::random rng1(12345);
        AT::util::random rng2(12345);
        
        for (int i = 0; i < 10; ++i) {
            REQUIRE(rng1.get<int>(0, 100) == rng2.get<int>(0, 100));
            REQUIRE(rng1.get<f32>(0.0f, 1.0f) == rng2.get<f32>(0.0f, 1.0f));
        }
    }
    
    SECTION("Different with different seeds") {
        // Two RNGs with different seeds should produce different sequences
        AT::util::random rng1(12345);
        AT::util::random rng2(67890);
        
        // It's possible they could be the same by chance, but very unlikely
        bool found_difference = false;
        for (int i = 0; i < 10; ++i) {
            if (rng1.get<int>(0, 100) != rng2.get<int>(0, 100)) {
                found_difference = true;
                break;
            }
        }
        
        REQUIRE(found_difference);
    }
}


TEST_CASE("Math functions work correctly", "[math]") {
    
#if 0
    unsigned int catch_seed = Catch::rngSeed();      // Get the seed from Catch2's random number generator
    AT::util::random rng(catch_seed);
#else
    AT::util::random rng;
#endif
    
    SECTION("min returns smaller value") {
        // Fixed tests
        REQUIRE(AT::math::min(5, 10) == 5);
        REQUIRE(AT::math::min(-5, 10) == -5);
        REQUIRE(AT::math::min(3.14f, 2.71f) == 2.71f);
        REQUIRE(AT::math::min('a', 'z') == 'a');
        
        for (int i = 0; i < 10; ++i) {                          // Random tests for integers
            int a = rng.get<int>(-100, 100);
            int b = rng.get<int>(-100, 100);
            int result = AT::math::min(a, b);
            REQUIRE(result <= a);
            REQUIRE(result <= b);
            REQUIRE((result == a || result == b));
        }
        
        for (int i = 0; i < 10; ++i) {                          // Random tests for f32s
            f32 a = rng.get<f32>(-100.0f, 100.0f);
            f32 b = rng.get<f32>(-100.0f, 100.0f);
            f32 result = AT::math::min(a, b);
            REQUIRE(result <= a);
            REQUIRE(result <= b);
            REQUIRE((result == a || result == b));
        }
    }

    SECTION("max returns larger value") {
        // Fixed tests
        REQUIRE(AT::math::max(5, 10) == 10);
        REQUIRE(AT::math::max(-5, 10) == 10);
        REQUIRE(AT::math::max(3.14f, 2.71f) == 3.14f);
        REQUIRE(AT::math::max('a', 'z') == 'z');
        
        for (int i = 0; i < 10; ++i) {                          // Random tests for integers
            int a = rng.get<int>(-100, 100);
            int b = rng.get<int>(-100, 100);
            int result = AT::math::max(a, b);
            REQUIRE(result >= a);
            REQUIRE(result >= b);
            REQUIRE((result == a || result == b));
        }
        
        for (int i = 0; i < 10; ++i) {                          // Random tests for f32s
            f32 a = rng.get<f32>(-100.0f, 100.0f);
            f32 b = rng.get<f32>(-100.0f, 100.0f);
            f32 result = AT::math::max(a, b);
            REQUIRE(result >= a);
            REQUIRE(result >= b);
            REQUIRE((result == a || result == b));
        }
    }
    
    SECTION("clamp function") {
        // Fixed tests
        REQUIRE(AT::math::clamp<int>(5, 0, 10) == 5);
        REQUIRE(AT::math::clamp<int>(-5, 0, 10) == 0);
        REQUIRE(AT::math::clamp<int>(15, 0, 10) == 10);
        REQUIRE(AT::math::clamp<f32>(3.14f, 0.0f, 5.0f) == 3.14f);
        
        // Random tests for integers
        for (int i = 0; i < 10; ++i) {
            int min_val = rng.get<int>(-50, 0);
            int max_val = rng.get<int>(1, 50);
            int value = rng.get<int>(-100, 100);
            
            int result = AT::math::clamp(value, min_val, max_val);
            
            if (value < min_val) {
                REQUIRE(result == min_val);
            } else if (value > max_val) {
                REQUIRE(result == max_val);
            } else {
                REQUIRE(result == value);
            }
        }
        
        // Random tests for f32s
        for (int i = 0; i < 10; ++i) {
            f32 min_val = rng.get<f32>(-50.0f, 0.0f);
            f32 max_val = rng.get<f32>(1.0f, 50.0f);
            f32 value = rng.get<f32>(-100.0f, 100.0f);
            
            f32 result = AT::math::clamp(value, min_val, max_val);
            
            if (value < min_val) {
                REQUIRE(result == min_val);
            } else if (value > max_val) {
                REQUIRE(result == max_val);
            } else {
                REQUIRE(result == value);
            }
        }
    }

    SECTION("lerp function") {
        // Fixed tests
        REQUIRE(AT::math::lerp(0.0f, 10.0f, 0.5f) == 5.0f);
        REQUIRE(AT::math::lerp(0, 10, 0.5f) == 5);
        REQUIRE(AT::math::lerp(-10.0f, 10.0f, 0.5f) == 0.0f);
        REQUIRE(AT::math::lerp(0.0f, 10.0f, 0.0f) == 0.0f);
        REQUIRE(AT::math::lerp(0.0f, 10.0f, 1.0f) == 10.0f);
        
        // Random tests for f32s
        for (int i = 0; i < 50; ++i) {
            f32 a = rng.get<f32>(-100.0f, 100.0f);
            f32 b = rng.get<f32>(-100.0f, 100.0f);
            f32 t = rng.get<f32>(0.0f, 1.0f);
            
            f32 result = AT::math::lerp(a, b, t);
            f32 expected = a + t * (b - a);
            
            // Allow for small f32ing point errors
            REQUIRE(std::abs(result - expected) < 0.0001f);
        }
        
        // Random tests for integers
        for (int i = 0; i < 50; ++i) {
            int a = rng.get<int>(-100, 100);
            int b = rng.get<int>(-100, 100);
            f32 t = rng.get<f32>(0.0f, 1.0f);
            
            int result = AT::math::lerp(a, b, t);
            int expected = static_cast<int>(a + t * (b - a));
            
            REQUIRE(result == expected);
        }
        
        // Edge cases
        for (int i = 0; i < 50; ++i) {
            f32 a = rng.get<f32>(-100.0f, 100.0f);
            f32 b = rng.get<f32>(-100.0f, 100.0f);
            
            REQUIRE(AT::math::lerp(a, b, 0.0f) == a);
            REQUIRE(AT::math::lerp(a, b, 1.0f) == b);
        }
    }

    SECTION("swap function") {
        // Fixed tests
        int a = 5, b = 10;
        AT::math::swap(a, b);
        REQUIRE(a == 10);
        REQUIRE(b == 5);

        f32 x = 3.14f, y = 2.71f;
        AT::math::swap(x, y);
        REQUIRE(x == 2.71f);
        REQUIRE(y == 3.14f);
        
        // Random tests for integers
        for (int i = 0; i < 10; ++i) {
            int original_a = rng.get<int>(-100, 100);
            int original_b = rng.get<int>(-100, 100);
            int a = original_a;
            int b = original_b;
            
            AT::math::swap(a, b);
            
            REQUIRE(a == original_b);
            REQUIRE(b == original_a);
        }
        
        // Random tests for f32s
        for (int i = 0; i < 10; ++i) {
            f32 original_x = rng.get<f32>(-100.0f, 100.0f);
            f32 original_y = rng.get<f32>(-100.0f, 100.0f);
            f32 x = original_x;
            f32 y = original_y;
            
            AT::math::swap(x, y);
            
            REQUIRE(x == original_y);
            REQUIRE(y == original_x);
        }
    }

    SECTION("abs function") {
        // Fixed tests
        REQUIRE(AT::math::abs(-5) == 5);
        REQUIRE(AT::math::abs(5) == 5);
        REQUIRE(AT::math::abs(-3.14f) == 3.14f);
        REQUIRE(AT::math::abs(3.14f) == 3.14f);
        
        // Random tests for integers
        for (int i = 0; i < 10; ++i) {
            int value = rng.get<int>(-100, 100);
            int result = AT::math::abs(value);
            
            REQUIRE(result >= 0);
            REQUIRE(result == (value < 0 ? -value : value));
        }
        
        // Random tests for f32s
        for (int i = 0; i < 10; ++i) {
            f32 value = rng.get<f32>(-100.0f, 100.0f);
            f32 result = AT::math::abs(value);
            
            REQUIRE(result >= 0.0f);
            REQUIRE(std::abs(result - (value < 0.0f ? -value : value)) < 0.0001f);
        }
    }

    SECTION("hash_combine function") {
        // Fixed tests
        std::size_t seed1 = 0;
        AT::math::hash_combine(seed1, 42, std::string("test"), 3.14);
        
        std::size_t seed2 = 0;
        AT::math::hash_combine(seed2, 42, std::string("test"), 3.14);
        
        // Same inputs should produce same hash
        REQUIRE(seed1 == seed2);
        
        std::size_t seed3 = 0;
        AT::math::hash_combine(seed3, 43, std::string("test"), 3.14);
        
        // Different inputs should produce different hash
        REQUIRE(seed1 != seed3);
        
        
        // Random tests
        for (int i = 0; i < 10; ++i) {
            std::size_t seed_a = 0;
            std::size_t seed_b = 0;
            
            // Generate random values
            int int_val = rng.get<int>(-100, 100);
            f32 f32_val = rng.get<f32>(-100.0f, 100.0f);
            std::string str_val = rng.get_string(10);
            
            // Same values should produce same hash
            AT::math::hash_combine(seed_a, int_val, f32_val, str_val);
            AT::math::hash_combine(seed_b, int_val, f32_val, str_val);
            REQUIRE(seed_a == seed_b);
            
            // Different values should produce different hashes
            std::size_t seed_c = 0;
            AT::math::hash_combine(seed_c, int_val + 1, f32_val, str_val);
            REQUIRE(seed_a != seed_c);          // FIXME: current collision rate is around:  1 in 13000
        }
    }
}



/* compile and run test:


clear && vendor/premake/premake5 gmake2 && make tests && echo "" && bin/Debug-linux-x86_64/tests/tests
use     -s      for verbose output

Run the tests multiple times:
./test_multi.sh

*/
