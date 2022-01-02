#include "pch.h"
#include "lockerfiles.h"
#include "securelib.h"

namespace fs = std::filesystem;

namespace securelib
{
	lockerfiles::lockerfiles(const std::wstring& rootDirPath, uint32_t room)
		: m_dirPath(rootDirPath)
		, m_room(room) // used for logging
	{
		if (m_dirPath.back() != fs::path::preferred_separator)
			m_dirPath += fs::path::preferred_separator;
		
		m_dirPath += std::to_wstring(room) + fs::path::preferred_separator;
	
		if (!fs::exists(m_dirPath))
			fs::create_directories(m_dirPath);
	}

	void lockerfiles::put(const std::wstring& filename, const std::vector<uint8_t>& bytes)
	{
		log(L"Files: PUT: " + std::to_wstring(m_room) + L" - " + filename);
		SaveFile(getFilePath(filename), bytes);
	}

	std::vector<uint8_t> lockerfiles::get(const std::wstring& filename)
	{
		log(L"Files: GET: " + std::to_wstring(m_room) + L" - " + filename);
		return LoadFile(getFilePath(filename));
	}

	void lockerfiles::del(const std::wstring& filename)
	{
		log(L"Files: DEL: " + std::to_wstring(m_room) + L" - " + filename);
		std::wstring filePath = getFilePath(filename);
		if (fs::exists(filePath))
			fs::remove(filePath);
	}

	std::wstring lockerfiles::dir()
	{
		log(L"Files: DIR: " + std::to_wstring(m_room));
		std::wstring dirResults;
		for (const auto& filePath : fs::directory_iterator(m_dirPath))
			dirResults += filePath.path().wstring().substr(m_dirPath.size()) + L"\n";
		return dirResults;
	}

	void lockerfiles::checkout()
	{
		log(L"Files: OUT: " + std::to_wstring(m_room));
		fs::remove_all(m_dirPath);
	}

	std::wstring lockerfiles::getFilePath(const std::wstring& filename) const
	{
		return m_dirPath + filename;
	}
}
