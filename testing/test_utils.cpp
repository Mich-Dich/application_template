// #define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>


#include "util/pch.h"
#include "util/math/math.h"
#include "util/math/random.h" 
#include "util/io/serializer_data.h"
#include "util/io/serializer_yaml.h"




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


/*

build and run only tests:
    clear && vendor/premake/premake5 gmake2 && make tests -j && echo "" && bin/Debug-linux-x86_64/tests/tests
    use     -s      for verbose output


Run the tests multiple times:
    ./test_multi.sh


build test and application (everything):
    clear; .vscode/build.sh

*/
