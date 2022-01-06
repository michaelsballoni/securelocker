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

		std::vector<uint8_t> bytes = 
			Decrypt(LoadFile(m_legerFilePath, false), m_password);

		std::string str8;
		str8.resize(bytes.size());
		memcpy(str8.data(), bytes.data(), bytes.size());

		std::wstring str = httplite::toWideStr(str8);
		std::vector<std::wstring> lines = httplite::Split(str, '\n');

		m_entries.clear();
		for (const auto& line : lines)
			m_entries.emplace_back(std::make_shared<legerentry>(line));
	}

	void lockerleger::registerClient(const std::wstring& name)
	{
		log(L"Leger: Register: " + name);
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

	std::string lockerleger::getRoom(const std::wstring& name)
	{
		for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			if ((*it)->name == name)
				return std::to_string((*it)->room) + " - " + (*it)->key;
		}
		throw std::runtime_error("Client not found");
	}

	void lockerleger::checkin(const std::wstring& name, uint32_t& room, std::string& key)
	{
		log(L"Leger: Checkin: " + name);
		std::lock_guard<std::mutex> lock(m_mutex);

		auto entry = getNameEntry(name);
		if (entry == nullptr)
			throw std::runtime_error("Client is not registered");
		if (entry->room > 0)
			throw std::runtime_error("Client already checked in");

		std::string totalUnique = UniqueStr();
		std::string ourUnique;
		for (size_t u = 0; u < 12; ++u) // only use the first bit, unique enough
		{
			if (u > 0 && (u % 3) == 0) // add dashes for human benefit
				ourUnique += "-";
			ourUnique += 'A' + (totalUnique[u] % 26); // humans like letters
		}

		room = getAvailableRoom();
		key = ourUnique;

		entry->room = room;
		entry->key = key;

		save();
		log(L"Leger: Checkin complete: " + name + L" - room " + std::to_wstring(room));
	}

	void lockerleger::checkout(const std::wstring& name, uint32_t& room)
	{
		log(L"Leger: Checkout: " + name);
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
		save();
		log(L"Leger: Checkout complete: " + name);
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

	uint32_t lockerleger::getAvailableRoom() const
	{
		// Get the sorted list of all rooms
		std::vector<uint32_t> rooms;
		for (const auto& legerentry : m_entries)
		{
			if (legerentry->room > 0)
				rooms.push_back(legerentry->room);
		}
		std::sort(rooms.begin(), rooms.end());

		// Walk the list looking for a gap in the sequence
		uint32_t last = 0;
		for (uint32_t cur : rooms)
		{
			uint32_t should = last + 1;
			if (cur > should) // past empty spot
				return should;
			last = cur;
		}

		// Failing that, go one after the end
		return last + 1;
	}

	lockerleger::legerentry::legerentry() : room(0) {}

	lockerleger::legerentry::legerentry(const std::wstring& _name, uint32_t _room, const std::string& _key)
		: room(_room)
		, name(_name)
		, key(_key)
	{}

	lockerleger::legerentry::legerentry(const std::wstring& str) // serialization
		: room(0)
	{
		std::vector<std::wstring> parts = httplite::Split(str, '\t');
		if (parts.size() != 3)
			throw std::runtime_error("Leger legerentry not right parts");

		int roomInt = _wtoi(parts[1].c_str());
		if (roomInt < 0)
			throw std::runtime_error("Leger legerentry invalid room number");

		name = parts[0];
		room = static_cast<uint32_t>(roomInt);
		key = httplite::toNarrowStr(parts[2]);
	}

	std::wstring lockerleger::legerentry::toString() const // serialization
	{
		return name + L"\t" + std::to_wstring(room) + L"\t" + httplite::toWideStr(key);
	}
}
