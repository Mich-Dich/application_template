# application\_template

A simple, cross-platform C++ application template with a graphical user interface (GUI) powered by ImGui and GLFW.

## 1. Overview

`application_template` provides a minimal but fully functional starting point for GUI-based C++ applications. It uses:

* **ImGui** for immediate-mode GUI widgets
* **GLFW** for window and input management
* **OpenGL** (configurable via `GL_renderer`)
* **Premake5** for build configuration

The template handles platform detection, dependency setup, and IDE integration out of the box.

## 2. Features

* Cross-platform support (Linux; Windows support coming soon)
* Automatic and manual build workflows
* IDE configuration for VSCode (default) and others
* Customizable dashboard module for runtime statistics or visuals
* Git submodule management for external dependencies

## 3. Prerequisites

* **Git** (with submodule support)
* **C++20** compatible compiler (e.g., `g++`, `clang++`)
* **Python 3** (for setup script)

## 4. Installation & Setup

### Run Setup Script

Use this template to create your own project, and clone it.

```bash
git clone <your_project>
cd <your_project>
```

It is recommended to change the `name` of your project in the `config/app_settings.yml`. This can ofcourse be change at any time, as long as the setup script is run afterwards

Execute the Python setup script to verify and install missing dependencies, configure the environment, and generate IDE/project files:

```bash
python3 setup.py
```

* The script detects your OS (Linux/Windows) and ensures all libraries and tools are available.
* Select your IDE when prompted (currently VSCode is supported).

## 5. Building the Application

### Automatic Build (VSCode)

If you selected **VSCode** during setup:

###### While in the VSCode editor:
* Press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>B</kbd> to build the application.
* Press <kbd>F5</kbd> to launch the application in the debugger after building.

### Manual Build

From the project root directory:

```bash
cd application_template
.vscode/build.sh      # Generate gmake2 files and run premake
make -j               # Compile with parallel jobs
```

If you modify `premake5.lua`, regenerate build files:

```bash
make clean && vendor/premake/premake5 gmake2
make -j
```

**Optional Bash Aliases** (add to your `~/.bashrc`):

```bash
alias app_build='cd ~/workspace/application_template && .vscode/build.sh && make -j'
alias app_run='app_build && bin/Debug-linux-x86_64/application_template/application_template'
```

Replace `~/workspace/application_template` with your project path.

## 6. Window Manager Integration

If you use a tiling window manager (e.g., `Krohnkite`), ImGui may spawn floating glfw windows with a prefix. Add the following rule to allow floating windows:

```
ISM -
```

This matches ImGui popup windows by their title prefix.

## 7. Usage

Run the compiled binary directly or via IDE. The main window displays ImGui controls—customize widgets in `application.cpp`.

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

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::Begin("Initialization", nullptr, window_flags);
    {

        const char* text = "Initializing...";
        const float target_font_size = 50.0f;
        ImVec2 base_text_size = ImGui::CalcTextSize(text);                                      // Calculate base text size at default font scale
        float scale = (base_text_size.y > 0) ? target_font_size / base_text_size.y : 1.0f;      // Calculate required scale to reach target font size
        ImVec2 available = ImGui::GetContentRegionAvail() * 0.9f;                               // Get available space with 10% margin
        ImVec2 scaled_size = base_text_size * scale;                                            // Calculate scaled text size
        
        if (scaled_size.x > available.x || scaled_size.y > available.y) {                       // Adjust scale if needed to fit available space
            float width_ratio = available.x / scaled_size.x;
            float height_ratio = available.y / scaled_size.y;
            scale *= (width_ratio < height_ratio) ? width_ratio : height_ratio;
        }
        
        // Set font scale and calculate final position
        ImGui::SetWindowFontScale(scale);
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImVec2 position = (ImGui::GetContentRegionAvail() - text_size) * 0.5f;
        
        ImGui::SetCursorPos(position);
        ImGui::TextUnformatted(text);
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
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

This project is licensed under the [MIT License](LICENSE).
