#pragma once

#include "lockerleger.h"

#include "HttpClient.h"
#include "HttpServer.h"

#include <atomic>
#include <string>

namespace securelib
{
	/// <summary>
	/// Manage all HTTP serving for the file locker server
	/// </summary>
	class lockerserver
	{
	public:
		lockerserver
		(
			uint16_t port,
			lockerleger& leger,
			const std::wstring& lockerRootDir
		);
		~lockerserver();

		void start();
		void stop();

		httplite::Response HandleRequest(const httplite::Request& request);

	private:
		httplite::HttpServer m_httpServer;

		lockerleger& m_leger;
		std::wstring m_lockerRootDir;
		std::atomic<int> m_lockerAuthNonce;
	};
}
