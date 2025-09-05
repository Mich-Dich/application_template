#pragma once

// -------------------------------------------------------------------------------------------------------------------------------------------
// Defines that have influence on system behavior
// -------------------------------------------------------------------------------------------------------------------------------------------

#define RENDER_API_OPENGL
// #define RENDER_API_VULKAN


// collect timing-data from every major function?
#define PROFILE								    1	// general
#define PROFILE_APPLICATION                     1
#define PROFILE_RENDERER                        0

// log assert and validation behaviour?
// NOTE - expr in assert/validation will still be executed
#define ENABLE_LOGGING_FOR_ASSERTS              1
#define ENABLE_LOGGING_FOR_VALIDATION           1


#define ASSET_EXTENTION			    ".atasset"      // Extension for asset files
#define PROJECT_EXTENTION    		".atproj"       // Extension for project files
#define CONFIG_FILE_EXTENSION   	".yml"        	// Extension for YAML config files
#define INI_FILE_EXTENSION      	".ini"          // Extension for INI config files
#define PROJECT_TEMP_DLL_PATH 		"build_DLL"     // Temporary directory for DLL builds

// Directory structure macros
#define METADATA_DIR            	"metadata"      // Directory for metadata files
#define CONFIG_DIR              	"config"        // Directory for configuration files
#define CONTENT_DIR             	"content"       // Directory for content files
#define SOURCE_DIR              	"src"           // Directory for source code

#define PROJECT_PATH				application::get().get_project_path()
#define PROJECT_NAME				application::get().get_project_data().name

#define ASSET_PATH					util::get_executable_path() / "assets"
