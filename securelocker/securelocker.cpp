#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "HttpClient.h"

#include "Core.h"
#pragma comment(lib, "httplite")
using namespace httplite;

#include "securelib.h"
#include "lockerhttp.h"
#pragma comment(lib, "securelib")
using namespace securelib;

#include <conio.h> // _getch()

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		// Validate command line usage
		if (argc < 5) 
		{
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
		if (verb == "DEL")
			verb = "DELETE";

		std::wstring originalFilename = argc < 6 ? L"" : argv[5];
		auto originalPath = fs::path(originalFilename);
		std::wstring parentPath = originalPath.has_parent_path() ? originalPath.parent_path() : "";
		std::wstring filename = originalPath.wstring().substr(parentPath.length());

		// Get the password from the user
		// The password is used for encrypting files before sending (PUT) to server
		// and decrypting files downloaded (GET) from the server
		// And that's it
		printf("Room Key: ");
		std::string key;
		char c;
		while ((c = _getch()) != '\n')
		{
			key += c;
			printf("*");
		}

		// Create our request, packing the payload for PUTs
		Request request;
		request.Verb = verb;
		request.Path.push_back(verb == "DIR" ? L"/" : filename);
		if (verb == "PUT")
		{
			Buffer payload;
			payload.Bytes = Encrypt(LoadFile(originalFilename), key);
			request.Payload = payload;
		}

		// Do the HTTP operation, including the nasty authentication
		HttpClient httpClient(server, port);
		Response response =
			securelib::issueClientHttpCommand
			(
				httpClient, 
				room, 
				key, 
				request
			);

		// Process the request and its successful response
		if (request.Verb == "GET")
		{
			printf("Saving...");
			SaveFile(originalPath, Decrypt(response.Payload->Bytes, key));
			printf("done!\n");
			printf("\nFile is now removed from the locker and stored locally.\n");
		}
		else if (request.Verb == "PUT")
		{
			printf("File is stored in the locker.\n");
		}
		else if (request.Verb == "DELETE")
		{
			printf("File has been removed from the locker.\n");
		}
		else if (request.Verb == "DIR")
		{
			std::wstring dirResult = response.Payload->ToString();
			if (dirResult.empty())
				printf("The locker contains no files.\n");
			else
				printf("Files in the locker:\n%S", dirResult.c_str());
		}
		else
			throw std::runtime_error(("Invalid request: " + request.Verb).c_str());
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
