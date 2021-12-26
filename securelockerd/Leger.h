#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace securelocker
{
	class Leger
	{
	public:
		// Leger schema: Name \t Room # \t Access Key
		// Read file into memory
		// Decrypt file using password
		// Parse file into leger object
		void load(const std::wstring& password);
		void save();

		// takes name and finds new room # and creates new key
		// room # found by scanning leger for empty slot, or at the end
		// new key created from hash of GUID and password and rand() and date/time
		void createEntry(const std::wstring& name, uint32_t& room, std::wstring& key);

		// matches name and room # and removes entry 
		void removeEntry(const std::wstring& name, uint32_t room);

	private:
		std::wstring m_password;

		struct Entry
		{
			std::wstring name;
			uint32_t room;
			std::wstring key;
		};
		std::vector<std::shared_ptr<Entry>> m_entries;
	};
}
