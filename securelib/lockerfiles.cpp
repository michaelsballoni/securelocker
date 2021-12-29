#include "pch.h"
#include "lockerfiles.h"
#include "securelib.h"

namespace fs = std::filesystem;

namespace securelib
{
	lockerfiles::lockerfiles(const std::wstring& rootDirPath, uint32_t room)
		: m_dirPath(rootDirPath)
	{
		if (m_dirPath.back() != fs::path::preferred_separator)
			m_dirPath += fs::path::preferred_separator;
		
		m_dirPath += std::to_wstring(room) + fs::path::preferred_separator;
	
		if (!fs::exists(m_dirPath))
			fs::create_directories(m_dirPath);
	}

	void lockerfiles::put(const std::wstring& filename, const std::vector<uint8_t>& bytes)
	{
		SaveFile(getFilePath(filename), bytes);
	}

	std::vector<uint8_t> lockerfiles::get(const std::wstring& filename)
	{
		return LoadFile(getFilePath(filename));
	}

	void lockerfiles::del(const std::wstring& filename)
	{
		std::wstring filePath = getFilePath(filename);
		if (fs::exists(filePath))
			fs::remove(filePath);
	}

	std::wstring lockerfiles::dir()
	{
		std::wstring dirResults;
		if (fs::exists(m_dirPath))
		{
			for (const auto& filePath : fs::directory_iterator(m_dirPath))
				dirResults += filePath.path().wstring().substr(m_dirPath.size()) + L"\n";
		}
		return dirResults;
	}

	std::wstring lockerfiles::getFilePath(const std::wstring& filename) const
	{
		return m_dirPath + filename;
	}
}