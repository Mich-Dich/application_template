// #define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>


#include "util/pch.h"
#include "util/data_structures/data_types.h"
#include "util/data_structures/deletion_queue.h"
#include "util/data_structures/type_deletion_queue.h"
#include "util/math/math.h"
#include "util/math/random.h" 
#include "util/io/serializer_data.h"
#include "util/io/serializer_yaml.h"
#include "util/io/serializer_binary.h"
#include "util/timing/stopwatch.h"



// ==============================================================================================================================
// RANDOM
// ==============================================================================================================================

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


// ==============================================================================================================================
// MATH
// ==============================================================================================================================

TEST_CASE("Math functions", "[math]") {
    
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

// ==============================================================================================================================
// LOGGER
// ==============================================================================================================================

TEST_CASE("Logger Basic Functionality", "[logger]") {
    // Create a temporary directory for test logs
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "logger_test";
    std::filesystem::create_directories(test_dir);
    
    SECTION("Initialization and Shutdown") {
        REQUIRE(AT::logger::init("[$T] $L: $C", false, test_dir, "test_basic.log"));
        REQUIRE_NOTHROW(AT::logger::shutdown());
    }
    
    SECTION("Basic Logging") {
        REQUIRE(AT::logger::init("$L: $C", false, test_dir, "test_basic.log"));
        
        LOG_Info("Test info message");
        LOG_Warn("Test warning message");
        LOG_Error("Test error message");
        
        REQUIRE_NOTHROW(AT::logger::shutdown());
        
        // Verify log file content
        std::ifstream log_file(test_dir / "test_basic.log");
        std::stringstream buffer;
        buffer << log_file.rdbuf();
        std::string content = buffer.str();
        
        REQUIRE(content.find("INFO: Test info message") != std::string::npos);
        REQUIRE(content.find("WARN: Test warning message") != std::string::npos);
        REQUIRE(content.find("ERROR: Test error message") != std::string::npos);
    }
    
    SECTION("Format Changes") {
        REQUIRE(AT::logger::init("$L: $C", false, test_dir, "test_format.log"));
        
        LOG_Info("Message with initial format");
        
        AT::logger::set_format("$T $L: $C");
        LOG_Info("Message with new format");
        
        AT::logger::use_previous_format();
        LOG_Info("Message with previous format");
        
        REQUIRE_NOTHROW(AT::logger::shutdown());
    }
    
    SECTION("Thread Label Registration") {
        REQUIRE(AT::logger::init("[$Q] $L: $C", false, test_dir, "test_thread_labels.log"));
        
        AT::logger::register_label_for_thread("MainThread");
        LOG_Info("Message from labeled thread");
        
        AT::logger::unregister_label_for_thread();
        LOG_Info("Message after unregistering");
        
        REQUIRE_NOTHROW(AT::logger::shutdown());
    }
    
    // Clean up
    std::filesystem::remove_all(test_dir);
}


TEST_CASE("Logger Multi-threading", "[logger][multithreading]") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "logger_mt_test";
    std::filesystem::create_directories(test_dir);
    
    REQUIRE(AT::logger::init("[$T] [$Q] $L: $C", false, test_dir, "test_multithread.log"));
    
    const int num_threads = 10;
    const int messages_per_thread = 50;
    std::vector<std::thread> threads;
    std::atomic<int> messages_logged(0);
    
    SECTION("Concurrent Logging") {
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([i, &messages_logged]() {
                std::string thread_name = "Thread" + std::to_string(i);
                AT::logger::register_label_for_thread(thread_name);
                
                for (int j = 0; j < messages_per_thread; j++) {
                    LOG_Info("Message " + std::to_string(j) + " from " + thread_name);
                    messages_logged++;
                }
                
                AT::logger::unregister_label_for_thread();
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        REQUIRE(messages_logged == num_threads * messages_per_thread);
    }
    
    REQUIRE_NOTHROW(AT::logger::shutdown());
    
    // Verify all messages were logged
    std::ifstream log_file(test_dir / "test_multithread.log");
    std::stringstream buffer;
    buffer << log_file.rdbuf();
    std::string content = buffer.str();
    
    for (int i = 0; i < num_threads; i++) {
        std::string thread_name = "Thread" + std::to_string(i);
        REQUIRE(content.find(thread_name) != std::string::npos);
    }
    
    // Clean up
    std::filesystem::remove_all(test_dir);
}


TEST_CASE("Logger Stress Test", "[logger][stress]") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "logger_stress_test";
    std::filesystem::create_directories(test_dir);
    
    // Use a smaller buffer to test buffer flushing
    REQUIRE(AT::logger::init("$L: $C", false, test_dir, "test_stress.log"));
    AT::logger::set_buffer_size(1024); // 1KB buffer
    AT::logger::set_buffer_threshold(AT::logger::severity::Warn); // Only buffer up to Warn
    
    const int num_threads = 8;
    const int messages_per_thread = 1000; // 8000 total messages
    std::vector<std::thread> threads;
    std::atomic<int> messages_logged(0);
    
    SECTION("High Volume Logging") {
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([i, &messages_logged]() {
                for (int j = 0; j < messages_per_thread; j++) {
                    // Mix different log levels
                    if (j % 10 == 0) {
                        LOG_Error("Error message " + std::to_string(j));
                    } else if (j % 5 == 0) {
                        LOG_Warn("Warning message " + std::to_string(j));
                    } else if (j % 3 == 0) {
                        LOG_Info("Info message " + std::to_string(j));
                    } else {
                        LOG_Debug("Debug message " + std::to_string(j));
                    }
                    messages_logged++;
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        REQUIRE(messages_logged == num_threads * messages_per_thread);
    }
    
    REQUIRE_NOTHROW(AT::logger::shutdown());
    
    // Verify log file exists and has content
    std::ifstream log_file(test_dir / "test_stress.log");
    REQUIRE(log_file.good());
    
    std::stringstream buffer;
    buffer << log_file.rdbuf();
    std::string content = buffer.str();
    
    // Should contain messages from all levels
    REQUIRE(content.find("ERROR:") != std::string::npos);
    REQUIRE(content.find("WARN:") != std::string::npos);
    REQUIRE(content.find("INFO:") != std::string::npos);
    REQUIRE(content.find("DEBUG:") != std::string::npos);
    
    std::filesystem::remove_all(test_dir);                          // Clean up
}


TEST_CASE("Logger Exception Handling", "[logger][exception]") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "logger_exception_test";
    std::filesystem::create_directories(test_dir);
    
    REQUIRE(AT::logger::init("$L: $C", false, test_dir, "test_exception.log"));
    
    SECTION("Logged Exception") {
        try {
            throw AT::logger::logged_exception(__FILE__, __FUNCTION__, __LINE__, std::this_thread::get_id(), "Test exception message");
            REQUIRE(false);                                         // Should not reach here
        } catch (const AT::logger::logged_exception& e) {
            REQUIRE(std::string(e.what()) == "Test exception message");
        }
    }
    
    REQUIRE_NOTHROW(AT::logger::shutdown());
    
    // Verify exception was logged
    std::ifstream log_file(test_dir / "test_exception.log");
    std::stringstream buffer;
    buffer << log_file.rdbuf();
    std::string content = buffer.str();
    
    REQUIRE(content.find("Test exception message") != std::string::npos);
    REQUIRE(content.find("ERROR:") != std::string::npos);
    
    std::filesystem::remove_all(test_dir);                          // Clean up
}

// ==============================================================================================================================
// YAML SERIALIZER
// ==============================================================================================================================

TEST_CASE("YAML Serializer - Basic Types", "[serializer][yaml]") {
    
    // use temporary directory for generated file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path test_file = temp_dir / "test_basic.yml";
    
    if (std::filesystem::exists(test_file))             // Ensure clean state
        std::filesystem::remove(test_file);
    
    // Test data
    int             test_int = 42,          loaded_int = 0;
    float           test_float = 3.14f,     loaded_float = 0.f;
    std::string     test_string = "hello",  loaded_string;          // leave uninitialized for test
    bool            test_bool = true,       loaded_bool;            // leave uninitialized for test

    // Serialize
    {
        AT::serializer::yaml(test_file, "basic_data", AT::serializer::option::save_to_file)
            .entry("test_int", test_int)
            .entry(KEY_VALUE(test_float))           // test macro with initialized
            .entry(KEY_VALUE(test_string))          // test macro with uninitialized
            .entry("test_bool", test_bool);
    }
    
    // Deserialize and verify
    {
        AT::serializer::yaml(test_file, "basic_data", AT::serializer::option::load_from_file)
            .entry("test_int", loaded_int)
            .entry("test_float", loaded_float)
            .entry("test_string", loaded_string)
            .entry("test_bool", loaded_bool);
    }

    REQUIRE(loaded_int == test_int);
    REQUIRE(loaded_float == Catch::Approx(test_float));
    REQUIRE(loaded_string == test_string);
    REQUIRE(loaded_bool == test_bool);
}


TEST_CASE("YAML Serializer - Complex Nested Structures", "[serializer][yaml]") {
    
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_complex.yml";
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    struct nested_config {
        struct sub_section {
            int value = 42;
            std::string name = "test";
        } subsection;
        
        std::vector<std::string> items = {"a", "b", "c"};
        std::unordered_map<std::string, int> mapping = {{"x", 1}, {"y", 2}};
    } test_config, loaded_config;

    {
        AT::serializer::yaml(test_file, "complex_data", AT::serializer::option::save_to_file)
            .sub_section("nested", [&](AT::serializer::yaml& yaml) {
                yaml.sub_section("subsection", [&](AT::serializer::yaml& yaml2) {
                    yaml2.entry("value", test_config.subsection.value)
                          .entry("name", test_config.subsection.name);
                })
                .entry("items", test_config.items)
                .unordered_map("mapping", test_config.mapping);
            });
    }

    {
        AT::serializer::yaml(test_file, "complex_data", AT::serializer::option::load_from_file)
            .sub_section("nested", [&](AT::serializer::yaml& yaml) {
                yaml.sub_section("subsection", [&](AT::serializer::yaml& yaml2) {
                    yaml2.entry("value", loaded_config.subsection.value)
                          .entry("name", loaded_config.subsection.name);
                })
                .entry("items", loaded_config.items)
                .unordered_map("mapping", loaded_config.mapping);
            });
    }

    REQUIRE(loaded_config.subsection.value == test_config.subsection.value);
    REQUIRE(loaded_config.subsection.name == test_config.subsection.name);
    REQUIRE(loaded_config.items == test_config.items);
    REQUIRE(loaded_config.mapping == test_config.mapping);
}


TEST_CASE("YAML Serializer - Multiple Sections", "[serializer][yaml]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_multisection.yml";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    int section1_value = 10, section2_value = 20;
    int loaded_section1_value = 0, loaded_section2_value = 0;

    {
        AT::serializer::yaml(test_file, "section1", AT::serializer::option::save_to_file)
            .entry("value", section1_value);
    }

    {
        AT::serializer::yaml(test_file, "section2", AT::serializer::option::save_to_file)
            .entry("value", section2_value);
    }

    {
        AT::serializer::yaml(test_file, "section1", AT::serializer::option::load_from_file)
            .entry("value", loaded_section1_value);
    }

    {
        AT::serializer::yaml(test_file, "section2", AT::serializer::option::load_from_file)
            .entry("value", loaded_section2_value);
    }

    REQUIRE(loaded_section1_value == section1_value);
    REQUIRE(loaded_section2_value == section2_value);
}


TEST_CASE("YAML Serializer - Special Characters", "[serializer][yaml]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_special_chars.yml";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    std::string test_string = "Line\nBreak\tTab\\Backslash\"Quote", loaded_string;

    {
        AT::serializer::yaml(test_file, "special_data", AT::serializer::option::save_to_file)
            .entry("special_string", test_string);
    }

    {
        AT::serializer::yaml(test_file, "special_data", AT::serializer::option::load_from_file)
            .entry("special_string", loaded_string);
    }

    REQUIRE(loaded_string == test_string);
}


TEST_CASE("YAML Serializer - Large Data", "[serializer][yaml]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_large.yml";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    const int NUM_ITEMS = 1000;
    std::vector<int> large_vector(NUM_ITEMS);
    std::iota(large_vector.begin(), large_vector.end(), 0); // Fill with 0..999
    
    std::vector<int> loaded_vector;

    {
        AT::serializer::yaml(test_file, "large_data", AT::serializer::option::save_to_file)
            .entry("large_array", large_vector);
    }

    {
        AT::serializer::yaml(test_file, "large_data", AT::serializer::option::load_from_file)
            .entry("large_array", loaded_vector);
    }

    REQUIRE(loaded_vector == large_vector);
}


TEST_CASE("YAML Serializer - Non-Existing Keys", "[serializer][yaml]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_missing.yml";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    int existing_value = 42;
    int loaded_existing = 0, loaded_missing = 100;

    {
        AT::serializer::yaml(test_file, "missing_data", AT::serializer::option::save_to_file)
            .entry("existing_key", existing_value);
    }

    {
        AT::serializer::yaml(test_file, "missing_data", AT::serializer::option::load_from_file)
            .entry("existing_key", loaded_existing)
            .entry("missing_key", loaded_missing);  // Should not change loaded_missing
    }

    REQUIRE(loaded_existing == existing_value);
    REQUIRE(loaded_missing == 100); // Should remain unchanged
}

// ==============================================================================================================================
// BINARY SERIALIZER
// ==============================================================================================================================

TEST_CASE("Binary Serializer - Basic Types", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_basic.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    // Test data
    int test_int = 42, loaded_int = 0;
    float test_float = 3.14f, loaded_float = 0.f;
    bool test_bool = true, loaded_bool = false;
    double test_double = 2.71828, loaded_double = 0.0;

    // Serialize
    {
        AT::serializer::binary(test_file, "basic_data", AT::serializer::option::save_to_file)
            .entry(test_int)
            .entry(test_float)
            .entry(test_bool)
            .entry(test_double);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "basic_data", AT::serializer::option::load_from_file)
            .entry(loaded_int)
            .entry(loaded_float)
            .entry(loaded_bool)
            .entry(loaded_double);
    }

    REQUIRE(loaded_int == test_int);
    REQUIRE(loaded_float == Catch::Approx(test_float));
    REQUIRE(loaded_bool == test_bool);
    REQUIRE(loaded_double == Catch::Approx(test_double));
}


TEST_CASE("Binary Serializer - Strings", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_strings.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    std::string test_string = "Hello, World!", loaded_string;
    std::string test_empty_string = "", loaded_empty_string;
    std::string test_long_string(1000, 'A'), loaded_long_string;

    // Serialize
    {
        AT::serializer::binary(test_file, "string_data", AT::serializer::option::save_to_file)
            .entry(test_string)
            .entry(test_empty_string)
            .entry(test_long_string);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "string_data", AT::serializer::option::load_from_file)
            .entry(loaded_string)
            .entry(loaded_empty_string)
            .entry(loaded_long_string);
    }

    REQUIRE(loaded_string == test_string);
    REQUIRE(loaded_empty_string == test_empty_string);
    REQUIRE(loaded_long_string == test_long_string);
}


TEST_CASE("Binary Serializer - File Paths", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_paths.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    std::filesystem::path test_path = "/some/test/path/file.txt";
    std::filesystem::path test_relative_path = "relative/path";
    std::filesystem::path loaded_path, loaded_relative_path;

    // Serialize
    {
        AT::serializer::binary(test_file, "path_data", AT::serializer::option::save_to_file)
            .entry(test_path)
            .entry(test_relative_path);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "path_data", AT::serializer::option::load_from_file)
            .entry(loaded_path)
            .entry(loaded_relative_path);
    }

    REQUIRE(loaded_path == test_path);
    REQUIRE(loaded_relative_path == test_relative_path);
}


TEST_CASE("Binary Serializer - Vectors", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_vectors.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    std::vector<int> test_int_vec = {1, 2, 3, 4, 5};
    std::vector<float> test_float_vec = {1.1f, 2.2f, 3.3f};
    std::vector<std::string> test_string_vec = {"one", "two", "three"};
    
    std::vector<int> loaded_int_vec;
    std::vector<float> loaded_float_vec;
    std::vector<std::string> loaded_string_vec;

    // Serialize
    {
        AT::serializer::binary(test_file, "vector_data", AT::serializer::option::save_to_file)
            .entry(test_int_vec)
            .entry(test_float_vec)
            .entry(test_string_vec);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "vector_data", AT::serializer::option::load_from_file)
            .entry(loaded_int_vec)
            .entry(loaded_float_vec)
            .entry(loaded_string_vec);
    }

    REQUIRE(loaded_int_vec == test_int_vec);
    REQUIRE(loaded_float_vec == test_float_vec);
    REQUIRE(loaded_string_vec == test_string_vec);
}


TEST_CASE("Binary Serializer - Arrays", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_arrays.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    const size_t array_size = 5;
    int* test_array = (int*)malloc(array_size * sizeof(int));
    for (size_t i = 0; i < array_size; i++) {
        test_array[i] = static_cast<int>(i * 10);
    }

    int* loaded_array = nullptr;

    // Serialize
    {
        AT::serializer::binary(test_file, "array_data", AT::serializer::option::save_to_file)
            .array(test_array, array_size);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "array_data", AT::serializer::option::load_from_file)
            .array(loaded_array, array_size);
    }

    for (size_t i = 0; i < array_size; i++) {
        REQUIRE(loaded_array[i] == test_array[i]);
    }

    // Clean up
    free(test_array);
    free(loaded_array);
}


TEST_CASE("Binary Serializer - Option Getter", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_option.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    // Test save option
    AT::serializer::binary saver(test_file, "option_test", AT::serializer::option::save_to_file);
    REQUIRE(saver.get_option() == AT::serializer::option::save_to_file);

    // Test load option
    AT::serializer::binary loader(test_file, "option_test", AT::serializer::option::load_from_file);
    REQUIRE(loader.get_option() == AT::serializer::option::load_from_file);
}


TEST_CASE("Binary Serializer - Large Data", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_large.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    const size_t large_size = 10000;
    std::vector<int> large_vector(large_size);
    for (size_t i = 0; i < large_size; i++) {
        large_vector[i] = static_cast<int>(i);
    }

    std::vector<int> loaded_vector;

    // Serialize
    {
        AT::serializer::binary(test_file, "large_data", AT::serializer::option::save_to_file)
            .entry(large_vector);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "large_data", AT::serializer::option::load_from_file)
            .entry(loaded_vector);
    }

    REQUIRE(loaded_vector.size() == large_size);
    for (size_t i = 0; i < large_size; i++) {
        REQUIRE(loaded_vector[i] == large_vector[i]);
    }
}


TEST_CASE("Binary Serializer - Mixed Data", "[serializer][binary]") {
    std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_mixed.bin";
    
    if (std::filesystem::exists(test_file))
        std::filesystem::remove(test_file);

    int test_int = 42;
    float test_float = 3.14f;
    std::string test_string = "test";
    std::vector<int> test_vector = {1, 2, 3};
    std::filesystem::path test_path = "/some/path";

    int loaded_int = 0;
    float loaded_float = 0.f;
    std::string loaded_string;
    std::vector<int> loaded_vector;
    std::filesystem::path loaded_path;

    // Serialize
    {
        AT::serializer::binary(test_file, "mixed_data", AT::serializer::option::save_to_file)
            .entry(test_int)
            .entry(test_float)
            .entry(test_string)
            .entry(test_vector)
            .entry(test_path);
    }

    // Deserialize and verify
    {
        AT::serializer::binary(test_file, "mixed_data", AT::serializer::option::load_from_file)
            .entry(loaded_int)
            .entry(loaded_float)
            .entry(loaded_string)
            .entry(loaded_vector)
            .entry(loaded_path);
    }

    REQUIRE(loaded_int == test_int);
    REQUIRE(loaded_float == Catch::Approx(test_float));
    REQUIRE(loaded_string == test_string);
    REQUIRE(loaded_vector == test_vector);
    REQUIRE(loaded_path == test_path);
}

// ==============================================================================================================================
// STOPWATCH
// ==============================================================================================================================

TEST_CASE("Stopwatch functionality", "[stopwatch][timing]") {

    SECTION("Basic timing functionality") {
        f32 elapsed_time = 0.0f;
        
        {
            AT::util::stopwatch sw(&elapsed_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } // Stopwatch stops automatically when going out of scope
        
        REQUIRE(elapsed_time > 0.0f);
        REQUIRE(elapsed_time >= 100.0f); // Should be at least 100ms
    }
    
    SECTION("Manual stop and restart") {
        f32 elapsed_time = 0.0f;
        AT::util::stopwatch sw(&elapsed_time);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        f32 first_measurement = sw.stop();
        
        REQUIRE(first_measurement > 0.0f);
        REQUIRE(first_measurement == elapsed_time);
        
        sw.restart();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        f32 second_measurement = sw.stop();
        
        REQUIRE(second_measurement > 0.0f);
        REQUIRE(second_measurement < first_measurement); // Should be shorter than first
    }
    
    SECTION("Different precision modes") {
        f32 ms_time = 0.0f;
        f32 us_time = 0.0f;
        f32 s_time = 0.0f;
        
        {
            AT::util::stopwatch ms_sw(&ms_time, AT::duration_precision::milliseconds);
            AT::util::stopwatch us_sw(&us_time, AT::duration_precision::microseconds);
            AT::util::stopwatch s_sw(&s_time, AT::duration_precision::seconds);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        REQUIRE(ms_time > 0.0f);
        REQUIRE(us_time > 0.0f);
        REQUIRE(s_time > 0.0f);
        
        // Verify relative scale of different precisions
        REQUIRE(us_time > ms_time); // Microseconds should be a larger number than milliseconds
        REQUIRE(s_time < 1.0f);     // 100ms should be 0.1 seconds
    }
    
    SECTION("Result getter") {
        f32 elapsed_time = 0.0f;
        AT::util::stopwatch sw(&elapsed_time);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        sw.stop();
        
        REQUIRE(sw.get_result() == elapsed_time);
        REQUIRE(sw.get_result() > 0.0f);
    }
    
    SECTION("Multiple consecutive measurements") {
        f32 times[3] = {0.0f, 0.0f, 0.0f};
        
        for (int i = 0; i < 3; i++) {
            AT::util::stopwatch sw(&times[i]);
            std::this_thread::sleep_for(std::chrono::milliseconds(20 * (i + 1)));
        }
        
        // Each measurement should be longer than the previous
        REQUIRE(times[0] > 0.0f);
        REQUIRE(times[1] > times[0]);
        REQUIRE(times[2] > times[1]);
    }
    
    SECTION("Very short measurements") {
        f32 short_time = 0.0f;
        
        {
            AT::util::stopwatch sw(&short_time);
            // Intentionally empty - measure the overhead of creating the stopwatch
        }
        
        // Even an empty block should register some time (though very small)
        REQUIRE(short_time >= 0.0f);
    }
}

// ==============================================================================================================================
// DELETION QUEUE
// ==============================================================================================================================

TEST_CASE("DeletionQueue - Basic Functionality", "[deletion_queue]") {
    
    AT::util::deletion_queue dq;
    
    SECTION("Empty queue flush") {
        REQUIRE_NOTHROW(dq.flush()); // Should not throw on empty queue
    }
    
    SECTION("Single function execution") {
        bool executed = false;
        dq.push_func([&]() { executed = true; });
        
        REQUIRE_FALSE(executed); // Should not execute before flush
        dq.flush();
        REQUIRE(executed); // Should execute after flush
    }
    
    SECTION("Multiple functions execution order") {
        std::vector<int> execution_order;
        
        dq.push_func([&]() { execution_order.push_back(1); });
        dq.push_func([&]() { execution_order.push_back(2); });
        dq.push_func([&]() { execution_order.push_back(3); });
        
        REQUIRE(execution_order.empty()); // Should not execute before flush
        dq.flush();
        
        // Functions should execute in reverse order (LIFO)
        REQUIRE(execution_order.size() == 3);
        REQUIRE(execution_order[0] == 3);
        REQUIRE(execution_order[1] == 2);
        REQUIRE(execution_order[2] == 1);
    }
}


TEST_CASE("DeletionQueue - Queue Clearing", "[deletion_queue]") {
    
    AT::util::deletion_queue dq;
    
    SECTION("Queue is cleared after flush") {
        int execution_count = 0;
        dq.push_func([&]() { execution_count++; });
        
        dq.flush();
        REQUIRE(execution_count == 1);
        
        // Second flush should not execute anything
        execution_count = 0;
        dq.flush();
        REQUIRE(execution_count == 0);
    }
    
    SECTION("Multiple flushes with new functions") {
        int total_executions = 0;
        
        // First set of functions
        dq.push_func([&]() { total_executions += 1; });
        dq.push_func([&]() { total_executions += 2; });
        dq.flush();
        REQUIRE(total_executions == 3);
        
        // Second set of functions
        dq.push_func([&]() { total_executions += 4; });
        dq.flush();
        REQUIRE(total_executions == 7);
    }
}


TEST_CASE("DeletionQueue - Exception Handling", "[deletion_queue]") {
    
    AT::util::deletion_queue dq;
    
    SECTION("Exception in one function doesn't stop others") {
        std::vector<int> executed_functions;
        
        dq.push_func([&]() { executed_functions.push_back(1); });
        dq.push_func([&]() { throw std::runtime_error("Test exception"); });
        dq.push_func([&]() { executed_functions.push_back(3); });
        
        // Should execute all functions even if one throws
        REQUIRE_THROWS_AS(dq.flush(), std::runtime_error);
        REQUIRE(executed_functions.size() == 2);
        REQUIRE(executed_functions[0] == 3); // Reverse order
        REQUIRE(executed_functions[1] == 1);
    }
    
    SECTION("Multiple exceptions") {
        dq.push_func([&]() { throw std::runtime_error("First exception"); });
        dq.push_func([&]() { throw std::logic_error("Second exception"); });
        
        // Should throw the first exception encountered (in reverse order)
        REQUIRE_THROWS_AS(dq.flush(), std::logic_error);
    }
}


TEST_CASE("DeletionQueue - Resource Management", "[deletion_queue]") {
    
    SECTION("Memory deallocation") {
        auto* ptr = new int(42);
        AT::util::deletion_queue dq;
        
        dq.push_func([ptr]() { delete ptr; });
        REQUIRE_NOTHROW(dq.flush());
        // ptr is now deleted - further access would be undefined behavior
    }
    
    SECTION("File handle cleanup") {
        std::ofstream test_file("test_temp.txt");
        test_file << "test content";
        test_file.close();
        
        AT::util::deletion_queue dq;
        bool file_deleted = false;
        
        dq.push_func([&]() {
            if (std::filesystem::exists("test_temp.txt")) {
                std::filesystem::remove("test_temp.txt");
                file_deleted = true;
            }
        });
        
        REQUIRE(std::filesystem::exists("test_temp.txt"));
        dq.flush();
        REQUIRE(file_deleted);
        REQUIRE_FALSE(std::filesystem::exists("test_temp.txt"));
    }
}


TEST_CASE("DeletionQueue - Complex Scenarios", "[deletion_queue]") {
    
    SECTION("Nested deletion queues") {
        AT::util::deletion_queue outer_dq;
        AT::util::deletion_queue inner_dq;
        std::vector<int> execution_order;
        
        inner_dq.push_func([&]() { execution_order.push_back(1); });
        outer_dq.push_func([&]() {
            execution_order.push_back(2);
            inner_dq.flush();
        });
        outer_dq.push_func([&]() { execution_order.push_back(3); });
        
        outer_dq.flush();
        
        // Execution order should be: 3, 2, then 1 (from inner_dq)
        REQUIRE(execution_order.size() == 3);
        REQUIRE(execution_order[0] == 3);
        REQUIRE(execution_order[1] == 2);
        REQUIRE(execution_order[2] == 1);
    }
    
    SECTION("Large number of functions") {
        AT::util::deletion_queue dq;
        const int NUM_FUNCTIONS = 1000;
        std::atomic<int> counter = 0;
        
        for (int i = 0; i < NUM_FUNCTIONS; i++) {
            dq.push_func([&]() { counter++; });
        }
        
        REQUIRE(counter == 0);
        dq.flush();
        REQUIRE(counter == NUM_FUNCTIONS);
    }
}


TEST_CASE("DeletionQueue - Thread Safety", "[deletion_queue][thread]") {

    SECTION("Concurrent push from multiple threads") {
        AT::util::deletion_queue dq;
        const int NUM_THREADS = 10;
        const int FUNCTIONS_PER_THREAD = 100;
        std::atomic<int> total_executions = 0;
        
        std::vector<std::thread> threads;
        for (int i = 0; i < NUM_THREADS; i++) {
            threads.emplace_back([&]() {
                for (int j = 0; j < FUNCTIONS_PER_THREAD; j++) {
                    dq.push_func([&]() { total_executions++; });
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        dq.flush();
        REQUIRE(total_executions == NUM_THREADS * FUNCTIONS_PER_THREAD);
    }
}




/*

build and run only tests:
    clear && vendor/premake/premake5 gmake2 && make tests -j && echo "" && bin/Debug-linux-x86_64/tests/tests
    use     -s      for verbose output


Run the tests multiple times:
    ./test_multi.sh


build test and application (everything):
    clear; .vscode/build.sh

*/
