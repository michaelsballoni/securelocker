#pragma once

namespace securelib
{
	/// <summary>
	/// Manage files contained in client lockers
	/// Each client locker is a flat list of files, no sub-directories
	/// Folks can use ZIP files if they need folders
	/// And they can password-protect those ZIP files 
	/// for a higher degree of security
	/// </summary>
	class lockerfiles
	{
	public:
		lockerfiles(const std::wstring& rootDirPath, uint32_t room);

		void put(const std::wstring& filename, const std::vector<uint8_t>& bytes);
		std::vector<uint8_t> get(const std::wstring& filename);
		void del(const std::wstring& filename);
		std::wstring dir();
		void checkout();

	private:
		std::wstring getFilePath(const std::wstring& filename) const;

	private:
		uint32_t m_room;
		std::wstring m_dirPath;
	};
}
