# application\_template

A simple, cross-platform C++ application template with a graphical user interface (GUI) powered by ImGui and GLFW.

## 1. Overview

`application_template` provides a functional starting point for GUI-based C++ applications. It uses:

* **ImGui** for immediate-mode GUI widgets
* **ImPlot**    <!-- TODO: add explanation -->
* **GLFW** for window and input management
* **OpenGL** (configurable via `GL_renderer`)

The template handles platform detection, dependency setup, and IDE integration out of the box.

## 2. Features

* Cross-platform support (Linux & Windows)
* Dual build system support: CMake and Premake5
* Automatic and manual build workflows
* IDE configuration for VSCode (default) and others
* Customizable dashboard module for runtime statistics or visuals
* Git submodule management for external dependencies

## 3. Prerequisites

* **Git** (with submodule support)
* **C++20** compatible compiler (e.g., `g++`, `clang++`)
* **Python 3** (for setup script)

## 4. Installation & Setup

### Clone Template

Use this template to create your own project, and clone it.

```bash
git clone <your_project>
cd <your_project>
```

### Run Setup Script

It is recommended to change the `name` of your project in the `config/app_settings.yml`. This can ofcourse be change at any time, as long as the setup script is run afterwards

Execute the Python setup script to verify and install missing dependencies, configure the environment, and generate IDE/project files:

```bash
python3 setup.py
```

* The script detects your OS (Linux/Windows) and ensures all libraries and tools are available.
* Select your build system when prompted (CMake or Premake5)
* Select your IDE when prompted (currently VSCode is supported).


### Build System Selection

The template supports two build systems:

#### CMake (Recommended)
- Modern, widely adopted build system
- Better IDE integration
- More configuration options
- Default selection

#### Premake5
- Simple Lua-based configuration
- Fast project generation
- Cross-platform project files

**Command-line options for build system selection:**
```bash
# Use CMake (skip selection prompt)
python3 setup.py -k

# Use Premake5 (skip selection prompt)  
python3 setup.py -p

# Interactive selection (default)
python3 setup.py
```

## 5. Building the Application

### Automatic Build (VSCode)

If you selected **VSCode** during setup:

###### While in the VSCode editor:
* Press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd> to build the application.
* Press <kbd>F5</kbd> to build and launch the application in the debugger.

### Manual Build - CMake

From the project root directory:

```bash
# Generate build files and build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 16

# Or create build directory manually
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j
```

### Manual Build - Premake5

From the project root directory:

```bash
# Generate build files (Linux)
./vendor/premake/premake5 gmake2

# Generate build files (Windows)  
vendor\premake\premake5.exe gmake2

# Build the project
gmake -j

# Clean build
gmake clean
```

## 6. Common Workflows

### Development Setup
```bash
# CMake with VSCode for debugging
python3 setup.py -k -V -C Debug

# Premake5 with VSCode
python3 setup.py -p -V -C Debug
```

### Release Build
```bash
# Generate release build files without building
python3 setup.py -k -C Release -n

# Or with Premake5
python3 setup.py -p -C Release -n
```

### Quick Setup
```bash
# CMake with VSCode, generate files only
python3 setup.py -k -V -n

# Full clean setup (removes previous builds)
python3 setup.py -c -k -V
```

## 7. Window Manager Integration

If you use a tiling window manager (e.g., `Krohnkite`), ImGui may spawn floating glfw windows with a prefix. Add the following rule to allow floating windows:

```
ISM -
```

This matches ImGui popup windows by their title prefix.

## 8. Usage

'F5' Runs the compiled binary directly or via IDE. 
The main window displays ImGui controls—customize widgets in `application.cpp`.

## Dashboard Module

The dashboard files (`dashboard.h` / `dashboard.cpp`) are entry points for adding custom UI panels or runtime metrics. Extend this module by implementing:

### Lifecycle Methods

1. `dashboard::init()`

   * Called once at application startup.
   * Load fonts, initialize data sources (e.g., attach to performance counters or networking subsystems).
   * Allocate any persistent buffers or GPU textures if needed.

2. `dashboard::draw(f32 deltaTime)`

   * Called every frame after your main rendering loop begins.
   * Use ImGui calls to create windows, plots, and widgets.
   * Query real-time data (e.g., FPS, memory usage) and display it.
   * Example:

     ```cpp
     void dashboard::draw() {
         ImGui::Begin("Performance Metrics");
         ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
         ImGui::PlotLines("Frame Times (ms)", frameTimeBuffer.data(), frameTimeBuffer.size(), 0,
                          "Frame Time", 0.0f, 50.0f, ImVec2(0, 80));
         ImGui::End();
     }
     ```

3. `dashboard::update(f32 deltaTime)` *(optional)*

   * If you need to sample or compute data at a different rate than the render loop.
   * For example, update network throughput or perform expensive calculations.

4. `dashboard::shutdown()` *(optional)*

   * Clean up resources allocated in `initialize()` (free textures, buffers).
   * Called on application exit.

### Long startup process (optional)

The template supports a simple, opt-in flow for long initialization/startup work. When enabled, the application will run the dashboard initialization in a separate thread and display a minimal "Initializing..." screen while the work completes.

**Enable it** by setting the following in `app_settings.yml` under the `general_settings` section:

```yaml
general_settings:
  long_startup_process: true
```

If `long_startup_process` is `true` the application will run `dashboard::init()` in a background thread and display the simple initialization UI until `init()` returns. This is intended for operations that may take several seconds (loading large resources, network requests, heavy warm-up tasks, etc.).

**Example `dashboard::init()`**

(Replace the demonstration sleep with your real initialization logic.)

```cpp
// init will be called when every system is initialized
bool dashboard::init() {

    // =========== Demonstrate a long startup process (just replace with custom logic) ===========
    bool long_startup_process = false;
    AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::app_settings), "general_settings", AT::serializer::option::load_from_file)
        .entry(KEY_VALUE(long_startup_process));

    if (long_startup_process)
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));  // 10s
    // ===========================================================================================

    return true;
}
```

**Initialization UI**

The UI shown while `dashboard::init()` runs is intentionally minimal and scales the text to the viewport. Example implementation:

```cpp
void dashboard::draw_init_UI(f32 delta_time) {

    // Overwrite with own UI or leave default
}
```

**Thread-safety notes & best practices**

* **Do not call ImGui functions from the background thread.** ImGui is not thread-safe — any UI calls must happen on the main/render thread. Use the background thread only for non-UI work (I/O, resource decoding, heavy computations).
* Synchronize shared data between the background thread and the main thread safely (mutexes, atomics, or lock-free queues). Update UI-visible state on the main thread.
* Keep the initialization UI simple — avoid complex widgets while the app is starting.
* Ensure `dashboard::init()` returns in a reasonable time or provides progress/status through a thread-safe flag if you want to show progress.

### Extending the Dashboard

* **Modular Panels**: Split different panels into private helper functions. Call them from `update()` and `draw()`:

  ```cpp
  void dashboard::draw() {
      draw_performance_panel();
      draw_network_panel();
      draw_custom_controls();
  }
  ```

* **Dynamic Layout**: Use `ImGui::BeginChild` and `ImGui::Columns` to arrange multiple panels in a single window.

* **Theming & Styling**: Customize colors and fonts via `ImGuiStyle` or push/pop style variables around specific widgets.

* **Input Handling**: Capture user inputs (e.g., toggling debug display) and store in Dashboard state to show or hide panels.

* **Unit Testing**: Abstract data collection into testable components. Inject mock data sources to verify plotting and formatting.

Example:

```cpp
void dashboard::draw() {
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    // Add your widgets here
    ImGui::End();
}
```

## Contributing

Contributions welcome! Please fork the repo and submit pull requests against `main`. Ensure you:

* Follow existing coding style (C++20, clang-format)
* Update documentation when adding features
* Test on Linux (other OS support TBD)

## License

This project is licensed under the [Apache 2.0 License](LICENSE).
