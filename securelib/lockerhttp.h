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
	/// <summary>
	/// Implement making an HTTP request to a file locker,
	/// yielding a response that can be handled by the client
	/// This function handles the challenge-response 
	/// authentication protocol, hiding the details behind this
	/// clean interface
	/// </summary>
	httplite::Response issueClientHttpCommand
	(
		httplite::HttpClient& client, 
		uint32_t room, 
		const std::string& key, 
		httplite::Request& request
	);

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
