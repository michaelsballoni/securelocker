#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace securelib
{
	class lockerleger
	{
	public: // interface
		lockerleger(const std::wstring& password, const std::wstring& legerFilePath);
		void load();

		// takes name and stashes it into leger with no room or key
		// users must register, out of band, before they can check in
		void registerName(const std::wstring& name);

		// takes name and finds entry and finds new room # and creates new key
		// room # found by scanning leger for empty slot, or at the end
		// new key created from hash of GUID and password and rand() and date/time
		void checkin(const std::wstring& name, uint32_t& room, std::string& key);

		// matches name and removes entry 
		void checkout(const std::wstring& name);

		struct legerentry
		{
			legerentry();
			legerentry(const std::wstring& _name, uint32_t _room, std::string _key);
			legerentry(const std::wstring& str);

			std::wstring toString() const;

			std::wstring name;
			uint32_t room;
			std::string key;
		};
		const std::vector<std::shared_ptr<legerentry>>& entries() const
		{
			return m_entries;
		}

	private: // implementation
		void save();
		bool isNameRegistered(const std::wstring& name) const;
		std::shared_ptr<legerentry> getNameEntry(const std::wstring& name) const;
		std::vector<uint32_t> getRooms() const;
		uint32_t getAvailableRoom() const;

	private: // state
		std::vector<std::shared_ptr<legerentry>> m_entries;

		mutable std::mutex m_mutex;

		std::string m_password;
		std::wstring m_legerFilePath;
	};
}
