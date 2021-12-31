#pragma once

#include "lockerleger.h"

#include "HttpClient.h"
#include "HttpServer.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace securelib
{
	httplite::Response issueClientHttpCommand
	(
		httplite::HttpClient& client, 
		uint32_t room, 
		const std::string& key, 
		httplite::Request& request
	);

	class lockerserver
	{
	public:
		lockerserver
		(
			uint16_t port,
			lockerleger& leger,
			const std::wstring& lockerRootDir, 
			std::atomic<int>& lockerAuthNonce
		);
		~lockerserver();

		void start();
		void stop();

		httplite::Response HandleRequests(const httplite::Request& request);

	private:
		httplite::HttpServer m_httpServer;

		lockerleger& m_leger;
		std::wstring m_lockerRootDir;
		std::atomic<int>& m_lockerAuthNonce;
	};
}
