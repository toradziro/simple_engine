#pragma once

#include <string>
#include <filesystem>

/*
 * Internal paths must start follow this notation "path/to/file.txt"
 */
using fs_path = std::filesystem::path;

std::string normalizePath(const fs_path& path);

struct File
{
	std::string toString() const;

	fs_path           m_virtualPath;
	std::vector<char> m_buffer;
};

class VirtualFS
{
public:
	explicit VirtualFS(std::string projectPath);

	File loadFile(const fs_path& path) const;
	File createFile(const fs_path& path) const;
	bool isFileExist(const fs_path& path) const;
	void writeFile(const File& file) const;
	fs_path virtualToNativePath(const fs_path& path) const;

private:
	fs_path m_projectPath;
};
