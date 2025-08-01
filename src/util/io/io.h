#pragma once


namespace AT::io {

	// Reads the content of a file into a vector of characters.
	std::string read_file(const std::filesystem::path& filepath);
		
	// Writes [content] to a file, overriding the previous content
	// @param [file_path] The path to the file to be written.
	// @param [content] The vector of characters to be written to the file.
	// @return [bool] true if the file is successfully written, false otherwise.
	bool write_file(const std::filesystem::path& file_path, const std::vector<char>& content);

	bool copy_file(const std::filesystem::path& full_path_to_file, const std::filesystem::path& darget_directory);
	
	// @brief Creates a directory at the specified path if it doesn't already exist.
	// @param [path] The path to the directory to be created.
	// @return [bool] Returns true if the directory is successfully created or already exists; false otherwise.
	bool create_directory(const std::filesystem::path& path);
	
	std::vector<std::string> get_processes_using_file(const std::wstring& filePath);

	bool is_directory(const std::filesystem::path& path);

	bool is_file(const std::filesystem::path& path);

	bool is_hidden(const std::filesystem::path& path);

	const std::filesystem::path get_absolute_path(const std::filesystem::path& path);

	std::vector<std::filesystem::path> get_files_in_dir(const std::filesystem::path& path);

	std::vector<std::filesystem::path> get_folders_in_dir(const std::filesystem::path& path);

	bool write_to_file(const char* data, const std::filesystem::path& filename);

}
