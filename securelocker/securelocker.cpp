#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "HttpClient.h"

#include <conio.h> // _getch()
#include <filesystem>
#include <iostream>

#include "Core.h"
#pragma comment(lib, "httplite")
using namespace httplite;

#include "securelib.h"
#pragma comment(lib, "securelib")
using namespace securelib;

namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		if (argc < 5) {
			printf("Usage: securelocker <server> <port> <room #> <put|get|delete|dir> [filename]\n");
			return 0;
		}
		
		std::string server = toNarrowStr(argv[1]);

		int port = _wtoi(argv[2]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %S\n", argv[2]);
			return 1;
		}

		int room = _wtoi(argv[3]);
		if (room <= 0) {
			printf("ERROR: Invalid room number: %S\n", argv[3]);
			return 1;
		}

		std::string verb = toNarrowStr(argv[4]);
		for (size_t s = 0; s < verb.size(); ++s)
			verb[s] = toupper(verb[s]);

		std::wstring originalFilename = argc < 6 ? L"" : argv[5];
		auto originalPath = fs::path(originalFilename);
		std::wstring parentPath = originalPath.has_parent_path() ? originalPath.parent_path() : "";
		std::wstring filename = originalPath.wstring().substr(parentPath.length());

		printf("Password: ");
		std::string password;
		char c;
		while ((c = _getch()) != '\n')
		{
			password += c;
			printf("*");
		}

		HttpClient client(server, port);

		Request request;
		request.Verb = verb;
		request.Path.push_back(verb == "DIR" ? L"/" : filename);
		if (verb == "PUT")
		{
			Buffer payload;
			payload.Bytes = Encrypt(LoadFile(originalFilename), password);
			request.Payload = payload;
		}
		request.Headers["Room-Number"] = std::to_string(room);

		bool gotChallenge = false;
		bool submittedChallenge = false;
		std::string challengePhrase;
		uint32_t nonce = 0;
		while (true)
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
							std::to_string(nonce)
						), 
						password
					);
				std::string challengeResponse =
					Hash(encryptedResponse.data(), encryptedResponse.size());
				request.Headers["Challenge-Response"] = challengeResponse;
				submittedChallenge = true;
			}

			printf("Issuing request...\n");
			Response response = client.ProcessRequest(request);
			uint16_t statusCode = response.GetStatusCode();
			if (statusCode == 200)
			{
				if (verb == "GET")
				{
					printf("Saving...");
					SaveFile(originalPath, response.Payload->Bytes);
					printf("done!\n");
					printf("\nFile is now removed from the locker and stored locally.\n");
				}
				else if (verb == "PUT")
				{
					printf("File is stored in the locker.\n");
				}
				else if (verb == "DELETE")
				{
					printf("File has been removed from the locker.\n");
				}
				else if (verb == "DIR")
				{
					printf("Files in the locker:\n%S", response.Payload->ToString().c_str());
				}
				else
				{
					printf("ERROR: Invalid request: %s\n", verb.c_str());
					return 1;
				}
				return 0;
			}
			else if (statusCode == 401)
			{
				if (submittedChallenge)
					return 1; // you get one shot

				gotChallenge = true;
				challengePhrase = response.Headers["Challenge-Phrase"];
				nonce = static_cast<uint32_t>(atoi(response.Headers["Nonce"].c_str()));
			}
			else
			{
				printf("ERROR: Server failure: %s\n", response.Status.c_str());
				return 1;
			}
		}
	}
#ifndef _DEBUG
	catch (const std::exception& exp)
	{
		printf("\ERROR: %s\n", exp.what());
		return 1;
	}
#endif
	return 0;
}
