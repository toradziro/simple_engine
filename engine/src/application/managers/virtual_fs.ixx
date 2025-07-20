export module virtual_fs;

import <string>;
import <filesystem>;
import <fstream>;
import <ostream>;
import <format>;
import engine_assert;

/*
 * Internal paths must start follow this notation "path/to/file.txt"
 */
export using fs_path = std::filesystem::path;

export std::string normalizePath(const fs_path& path)
{
	std::string res = path.string();
	size_t      startPos = 0;
	while ((startPos = res.find('\\', startPos)) != std::string::npos)
	{
		res.replace(startPos, 1, "/");
		startPos += 1;
	}
	return res;
}

export struct File
{
	std::string toString() const
	{
		std::string res;
		res.resize(m_buffer.size());
		memcpy(res.data(), m_buffer.data(), m_buffer.size());
		return res;
	}

	fs_path           m_virtualPath;
	std::vector<char> m_buffer;
};

export class VirtualFS
{
public:
	explicit VirtualFS(std::string projectPath)
	{
		if (projectPath.back() != '/' && projectPath.back() != '\\')
		{
			projectPath += "/";
		}
		m_projectPath = normalizePath(std::move(projectPath));
	}

	File loadFile(const fs_path& path) const
	{
		File readFile;
		readFile.m_virtualPath = path;
		fs_path nativeFilePath = virtualToNativePath(path);

		if (std::ifstream in(nativeFilePath, std::ios::in | std::ios::binary))
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();

			engineAssert(size != -1, std::format("Reading error in file: '{}'", nativeFilePath.generic_string()));
			auto& buffer = readFile.m_buffer;
			buffer.resize(size, 0);
			in.seekg(0, std::ios::beg);
			in.read(buffer.data(), buffer.size());
			in.close();
		}

		return readFile;
	}

	File createFile(const fs_path& path) const
	{
		File file{ .m_virtualPath = path };

		return file;
	}

	bool isFileExist(const fs_path& path) const
	{
		const fs_path nativePath = virtualToNativePath(path);
		return std::filesystem::exists(nativePath);
	}

	void writeFile(const File& file) const
	{
		const fs_path nativePath = virtualToNativePath(file.m_virtualPath);
		if (std::ofstream out(nativePath, std::ios::out | std::ios::trunc | std::ios::binary))
		{
			out.write(file.m_buffer.data(), file.m_buffer.size());
			out.close();
		}
	}

	fs_path virtualToNativePath(const fs_path& path) const
	{
		const auto res = m_projectPath / path;
		return normalizePath(res);
	}

private:
	fs_path m_projectPath;
};
