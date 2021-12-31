#include "pch.h"
#include "lockerleger.h"

#include "Core.h"
#include "securelib.h"

namespace securelib
{
	lockerleger::lockerleger(const std::wstring& password, const std::wstring& legerFilePath)
		: m_password(httplite::toNarrowStr(password))
		, m_legerFilePath(legerFilePath)
	{}

	void lockerleger::load()
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		std::vector<uint8_t> bytes = Decrypt(LoadFile(m_legerFilePath), m_password);

		std::string str8;
		str8.resize(bytes.size());
		memcpy(str8.data(), bytes.data(), bytes.size());

		std::wstring str = httplite::toWideStr(str8);
		std::vector<std::wstring> lines = httplite::Split(str, '\n');

		m_entries.clear();
		for (const auto& line : lines)
			m_entries.emplace_back(std::make_shared<legerentry>(line));
	}

	void lockerleger::registerName(const std::wstring& name)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if
		(
			name.find('\t') != std::wstring::npos 
			|| 
			name.find('\n') != std::wstring::npos
		)
		{
			throw std::runtime_error("Invalid client name");
		}

		if (isNameRegistered(name))
			throw std::runtime_error("Client already registered");

		m_entries.emplace_back(std::make_shared<legerentry>(name, 0, "_"));
		
		save();
	}

	void lockerleger::checkin(const std::wstring& name, uint32_t& room, std::string& key)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		auto entry = getNameEntry(name);
		if (entry == nullptr)
			throw std::runtime_error("Client is not registered");

		std::string totalUnique = UniqueStr();
		std::string userUnique;
		for (size_t u = 0; u <= 10; ++u)
		{
			if (u == 2 || u == 6) // phone number style
				userUnique += "-";
			userUnique += totalUnique[u];
		}

		room = getAvailableRoom();
		key = userUnique;

		entry->room = room;
		entry->key = key;

		save();
	}

	void lockerleger::checkout(const std::wstring& name, uint32_t& room)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		bool found = false;
		for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			if ((*it)->name == name)
			{
				found = true;
				room = (*it)->room;
				m_entries.erase(it);
				break;
			}
		}
		if (!found)
			throw std::runtime_error("Client not found");
		// FORNOW - Delete user's files directory
		save();
	}

	std::string lockerleger::getRoomKey(uint32_t room)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			if ((*it)->room == room)
				return (*it)->key;
		}
		throw std::runtime_error("Room not found");
	}

	// NOTE: Does not lock as mutex is not recursive, 
	//		 and only called by public functions that lock
	void lockerleger::save()
	{
		std::wstring str;
		for (const auto& legerentry : m_entries)
			str += legerentry->toString() + L"\n";

		std::string str8 = httplite::toNarrowStr(str);

		std::vector<uint8_t> bytes;
		bytes.resize(str8.size());
		memcpy(bytes.data(), str8.data(), str8.size());

		SaveFile(m_legerFilePath, Encrypt(bytes, m_password));
	}

	bool lockerleger::isNameRegistered(const std::wstring& name) const
	{
		return getNameEntry(name) != nullptr;
	}

	std::shared_ptr<lockerleger::legerentry> lockerleger::getNameEntry(const std::wstring& name) const
	{
		for (const auto& legerentry : m_entries)
		{
			if (legerentry->name == name)
				return legerentry;
		}
		return nullptr;
	}

	std::vector<uint32_t> lockerleger::getRooms() const
	{
		std::vector<uint32_t> rooms;
		for (const auto& legerentry : m_entries)
		{
			if (legerentry->room > 0)
				rooms.push_back(legerentry->room);
		}
		std::sort(rooms.begin(), rooms.end());
		return rooms;
	}

	uint32_t lockerleger::getAvailableRoom() const
	{
		std::vector<uint32_t> rooms = getRooms();
		uint32_t last = 0, max = 0;
		for (size_t r = 0; r < rooms.size(); ++r)
		{
			uint32_t cur = rooms[r];
			uint32_t should = last + 1;
			if (cur == should)
				return cur;
			last = cur;
			max = cur;
		}
		return max + 1;
	}

	lockerleger::legerentry::legerentry() : room(0) {}

	lockerleger::legerentry::legerentry(const std::wstring& _name, uint32_t _room, std::string _key)
		: room(_room)
		, name(_name)
		, key(_key)
	{}

	lockerleger::legerentry::legerentry(const std::wstring& str)
		: room(0)
	{
		std::vector<std::wstring> parts = httplite::Split(str, '\t');
		if (parts.size() != 3)
			throw std::runtime_error("Leger legerentry not right parts");

		name = parts[0];
		room = static_cast<uint32_t>(_wtoi(parts[1].c_str()));
		key = httplite::toNarrowStr(parts[2]);
	}

	std::wstring lockerleger::legerentry::toString() const
	{
		return name + L"\t" + std::to_wstring(room) + L"\t" + std::wstring(key.begin(), key.end());
	}
}
