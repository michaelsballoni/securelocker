#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace securelib
{
	/// <summary>
	/// Manage the leger of which clients are in which rooms with which room keys
	/// </summary>
	class lockerleger
	{
	public: // interface
		lockerleger(const std::wstring& password, const std::wstring& legerFilePath);
		void load();

		/// <summary>
		/// Take name and stash it in the leger with no room or key
		/// Clients must register, out of band, before they can check in
		/// </summary>
		void registerClient(const std::wstring& name);

		/// <summary>
		/// What room is a client staying in?
		/// </summary>
		std::string getRoom(const std::wstring& name);
		
		/// <summary>
		/// Takes name and finds entry and find new room # and creates new key
		/// </summary>
		void checkin(const std::wstring& name, uint32_t& room, std::string& key);

		/// <summary>
		/// Matche name and removes entry, returning the room number
		/// </summary>
		void checkout(const std::wstring& name, uint32_t& room);

		/// <summary>
		/// Get the key for a given room
		/// Used the HTTP authentication mechanism
		/// </summary>
		std::string getRoomKey(uint32_t room);

		struct legerentry
		{
			legerentry();
			legerentry(const std::wstring& _name, uint32_t _room, const std::string& _key);
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
		
		uint32_t getAvailableRoom() const;

	private: // state
		std::vector<std::shared_ptr<legerentry>> m_entries;

		mutable std::mutex m_mutex;

		const std::string m_password;
		const std::wstring m_legerFilePath;
	};
}
