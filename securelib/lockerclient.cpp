#include "pch.h"
#include "lockerclient.h"
#include "securelib.h"

using namespace httplite;

namespace securelib
{
	/// <summary>
	/// Implement making an HTTP request to a file locker,
	/// yielding a response that can be handled by the client
	/// This function handles the challenge-response 
	/// authentication protocol, hiding the details behind this
	/// clean interface
	/// </summary>
	static Response issueClientHttpCommand
	(
		httplite::HttpClient& client,
		uint32_t room,
		const std::string& key,
		httplite::Request& request
	)
	{
		trace(L"Client HTTP Command: " + std::to_wstring(room) + L" - " + toWideStr(request.Verb) + L" - " + request.Path[0]);
		request.Headers["X-Room-Number"] = std::to_string(room);

		bool gotChallenge = false;
		bool submittedChallenge = false;

		std::string challengePhrase;
		std::string challengeNonce;

		while (true) // process authentication challenges then the actual request
		{
			if (gotChallenge)
			{
				auto encryptedResponse =
					Encrypt
					(
						StrToVec
						(
							std::to_string(room) +
							challengePhrase +
							challengeNonce
						),
						key
					);
				std::string challengeResponse =
					Hash(encryptedResponse.data(), encryptedResponse.size());
				request.Headers["X-Challenge-Response"] = challengeResponse;
				submittedChallenge = true;
				trace("Got challenge, response: " + challengeResponse);
			}

			trace("Issuing request...");
			Response response = client.ProcessRequest(request);
			trace("Response: " + response.Status);
			uint16_t statusCode = response.GetStatusCode();
			if (statusCode / 100 == 2)
			{
				// Authentication is fine, so remove the auth headers 
				// to keep subsequent requests clean
				request.Headers.erase("X-Room-Number");
				request.Headers.erase("X-Challenge-Response");
				return response;
			}
			else if (statusCode / 100 == 4)
			{
				// no double-dipping, you get one shot
				// client expects a successful response, 
				// so we throw instead for return response
				if (submittedChallenge)
					throw std::runtime_error("Access denied.");

				gotChallenge = true;
				challengePhrase = response.Headers["X-Challenge-Phrase"];
				challengeNonce = response.Headers["X-Challenge-Nonce"];
				//trace("challengePhrase: " + challengePhrase);
				//trace("challengeNonce: " + challengeNonce);
			}
			else
				throw std::runtime_error(("Unregonized Server Response: " + response.Status).c_str());
		}
	}

	lockerclient::lockerclient(const std::string& server, uint16_t port, uint32_t room, const std::string& key)
		: m_client(server, port)
		, m_room(room)
		, m_key(key)
	{}

	Response lockerclient::doHttp(Request& request)
	{
		Response response =
			issueClientHttpCommand
			(
				m_client,
				m_room,
				m_key,
				request
			);
		if (response.GetStatusCode() / 100 != 2)
			throw std::runtime_error(("HTTP operation failed: " + response.Status).c_str());
		return response;
	}

	void lockerclient::put(const std::wstring& filename, const std::vector<uint8_t>& bytes)
	{
		Request request;
		request.Verb = "PUT";
		request.Path.push_back(filename);
		request.Payload.emplace(Encrypt(bytes, m_key));
		doHttp(request);
	}
	
	std::vector<std::wstring> lockerclient::dir()
	{
		Request request;
		request.Verb = "DIR";
		request.Path.push_back(L"/");
		Response response = doHttp(request);
		std::wstring dirResult =
			response.Payload.has_value() ? response.Payload->ToString() : L"";
		return Split(dirResult, L'\n');
	}

	std::vector<uint8_t> lockerclient::get(const std::wstring& filename)
	{
		Request request;
		request.Verb = "GET";
		request.Path.push_back(filename);
		Response response = doHttp(request);
		return Decrypt(response.Payload->Bytes, m_key);
	}

	void lockerclient::del(const std::wstring& filename)
	{
		Request request;
		request.Verb = "DELETE";
		request.Path.push_back(filename);
		doHttp(request);
	}
}
