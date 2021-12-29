#pragma once

namespace securelib
{
	class lockerfiles
	{
	public:
		lockerfiles(const std::wstring& rootDirPath, uint32_t room);

		void put(const std::wstring& filename, const std::vector<uint8_t>& bytes);
		std::vector<uint8_t> get(const std::wstring& filename);
		void del(const std::wstring& filename);
		std::wstring dir();

	private:
		std::wstring getFilePath(const std::wstring& filename) const;

	private:
		std::wstring m_dirPath;
	};
}