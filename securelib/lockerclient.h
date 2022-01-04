#pragma once

#include "HttpClient.h"

#include <string>
#include <vector>

namespace securelib
{
	class lockerclient
	{
	public:
		lockerclient(const std::string& server, uint16_t port, uint32_t room, const std::string& key);

		void put(const std::wstring& filename, const std::vector<uint8_t>& bytes);
		std::vector<std::wstring> dir();
		std::vector<uint8_t> get(const std::wstring& filename);
		void del(const std::wstring& filename);

	private:
		httplite::Response doHttp(httplite::Request& request);

	private:
		httplite::HttpClient m_client;
		uint32_t m_room;
		std::string m_key;
	};
}