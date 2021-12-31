#include "pch.h"
#include "lockerhttp.h"
#include "lockerfiles.h"
#include "securelib.h"

using namespace httplite;
using namespace securelib;

Response securelib::issueClientHttpCommand
(
	HttpClient& client,
	uint32_t room,
	const std::string& key,
	Request& request
)
{
#ifdef _DEBUG
	printf("%s %S\n", request.Verb.c_str(), request.Path[0].c_str());
#endif
	request.Headers["X-Room-Number"] = std::to_string(room);

	bool gotChallenge = false;
	bool submittedChallenge = false;

	std::string challengePhrase;
	std::string nonce;
		
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
						nonce
					),
					key
				);
			std::string challengeResponse =
				Hash(encryptedResponse.data(), encryptedResponse.size());
			request.Headers["X-Challenge-Response"] = challengeResponse;
#ifdef _DEBUG
			printf("Got challenge, response: %s\n", challengeResponse.c_str());
#endif
			submittedChallenge = true;
		}

#ifdef _DEBUG
		printf("Issuing request...\n");
#endif
		Response response = client.ProcessRequest(request);
#ifdef _DEBUG
		printf("Response: %s\n", response.Status.c_str());
#endif
		uint16_t statusCode = response.GetStatusCode();
		if (statusCode / 100 == 2)
		{
			// Authentication is fine, so remove the auth headers 
			// to keep requests clean
			request.Headers.erase("X-Room-Number");
			request.Headers.erase("X-Challenge-Response");
			return response;
		}
		else if (statusCode / 100 == 4)
		{
			if (submittedChallenge)
				throw std::runtime_error("Access denied.");

			gotChallenge = true;
			challengePhrase = response.Headers["X-Challenge-Phrase"];
			nonce = response.Headers["X-Challenge-Nonce"];
		}
		else
			throw std::runtime_error(("Server error: " + response.Status).c_str());
	}
}

static std::shared_ptr<Response> authServerHttpRequest // local helper function
(
	const Request& request,
	std::function<int()> nonceGen,
	std::function<std::string(uint32_t room)> keyGet
)
{
	// Make connection variables easier to work with
	auto& connVars = *request.ConnectionVariables;

	// Maybe the user is already authenticated
	if (connVars.find(L"Authenticated") != connVars.end())
		return nullptr;

	// Unpack the challenge connection vars
	auto roomIt = connVars.find(L"RoomNumber");
	auto challengeIt = connVars.find(L"ChallengePhrase");
	auto nonceIt = connVars.find(L"ChallengeNonce");
	if
		(
			roomIt == connVars.end()
			||
			challengeIt == connVars.end()
			||
			nonceIt == connVars.end()
			)
	{ // No challenge yet
		// Get the room number from the request and validate it
		// NOTE: This is the only time when the room number is read from the client
		//		 We can't allow clients to change which room they're talking about 
		//		 later and gain access to another room's contents, password or not
		auto roomRequestIt = request.Headers.find("X-Room-Number");
		if (roomRequestIt == request.Headers.end())
		{
			std::shared_ptr<Response> response = std::make_shared<Response>();
			response->Status = "403 Invalid Request";
			return response;
		}
		int roomInt = atoi(roomRequestIt->second.c_str());
		if (roomInt <= 0)
		{
			std::shared_ptr<Response> response = std::make_shared<Response>();
			response->Status = "403 Invalid Request";
			return response;
		}

		// Create the challenge
		std::string challenge = UniqueStr();
		std::string nonce = std::to_string(nonceGen());

		// Stash the challenge in connections vars
		connVars[L"RoomNumber"] = std::to_wstring(roomInt);
		connVars[L"ChallengePhrase"] = toWideStr(challenge);
		connVars[L"ChallengeNonce"] = toWideStr(nonce);

		// Return the challenget response
		std::shared_ptr<Response> response = std::make_shared<Response>();
		response->Status = "401 Access Denied";
		response->Headers["X-Challenge-Phrase"] = challenge;
		response->Headers["X-Challenge-None"] = nonce;
		return response;
	}
	else // we have a complete set of challenge connections vars
	{
		// Unpack the server challenge from the connection vars
		uint32_t room = static_cast<uint32_t>(_wtoi(roomIt->second.c_str()));
		std::string challengePhrase = toNarrowStr(challengeIt->second);
		std::string challengeNonce = toNarrowStr(nonceIt->second);

		// Erase the challenge connection vars to prevent clients from hammering
		// on the same challenge until they find something that works
		connVars.erase(roomIt);
		connVars.erase(challengeIt);
		connVars.erase(nonceIt);

		// Get the client's response to the server challenge
		std::string challengeClientResponse;
		{
			auto requestChallengeResponseIt = request.Headers.find("X-Challenge-Response");
			if (requestChallengeResponseIt == request.Headers.end())
			{
				std::shared_ptr<Response> response = std::make_shared<Response>();
				response->Status = "401 Access Denied";
				return response;
			}
			challengeClientResponse = requestChallengeResponseIt->second;
		}

		// Compute the server version of the challenge response
		auto encryptedLocalResponse =
			Encrypt
			(
				StrToVec
				(
					std::to_string(room) +
					challengePhrase +
					challengeNonce
				),
				keyGet(room)
			);
		std::string challengeLocalResponse =
			Hash(encryptedLocalResponse.data(), encryptedLocalResponse.size());
		if (challengeClientResponse == challengeLocalResponse) // user is granted access
		{
			connVars[L"Authenticated"] = L"true";
			connVars[L"RoomNumber"] = std::to_wstring(room);
			return nullptr;
		}
		else // user's response to challenge fails
		{
			std::shared_ptr<Response> response = std::make_shared<Response>();
			response->Status = "401 Access Denied";
			return response;
		}
	}

	// Unreachable
	//return nullptr;
}

namespace securelib
{
	lockerserver::lockerserver
	(
		uint16_t port,
		lockerleger& leger,
		const std::wstring& lockerRootDir,
		std::atomic<int>& lockerAuthNonce
	)
		: m_httpServer(port, [this](const Request& request) { return HandleRequests(request); })
		, m_leger(leger)
		, m_lockerRootDir(lockerRootDir)
		, m_lockerAuthNonce(lockerAuthNonce)
	{
	}

	lockerserver::~lockerserver()
	{
		stop();
	}

	void lockerserver::start()
	{
		m_httpServer.StartServing();
	}

	void lockerserver::stop()
	{
		m_httpServer.StopServing();
	}

	Response lockerserver::HandleRequests(const Request& request)
	{
#ifdef _DEBUG
		printf("%s %S\n", request.Verb.c_str(), Join(request.Path, L"/").c_str());
#endif
		// Authenticate the user
		auto& authNonce = m_lockerAuthNonce;
		auto& authLeger = m_leger;
		auto authResponse =
			authServerHttpRequest
			(
				request,
				[&authNonce]() { return authNonce.fetch_add(1); },
				[&authLeger](uint32_t room) { return authLeger.getRoomKey(room); }
			);
		if (authResponse != nullptr)
			return *authResponse;

		// Handle the request
		auto& connVars = *request.ConnectionVariables;
		uint32_t room = static_cast<uint32_t>(_wtoi(connVars[L"RoomNumber"].c_str()));
		lockerfiles files(m_lockerRootDir, room);
		Response response;
		if (request.Verb == "PUT")
		{
			files.put(request.Path[0], request.Payload->Bytes);
			response.Status = "204 File Stored";
		}
		else if (request.Verb == "GET")
		{
			response.Payload->Bytes = files.get(request.Path[0]);
		}
		else if (request.Verb == "DELETE")
		{
			files.del(request.Path[0]);
			response.Status = "204 File Deleted";
		}
		return response;
	}
}
