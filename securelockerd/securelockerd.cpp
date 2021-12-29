#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "lockerleger.h"
#include "HttpServer.h"

#include <conio.h> // _getch()
#include <iostream>

#pragma comment(lib, "httplite")
#pragma comment(lib, "securelib")

using namespace httplite;

Response HandleRequests(const Request& request);

std::shared_ptr<securelib::lockerleger> leger;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		if (argc != 3) {
			printf("Usage: securelockerd <port> <leger file path>\n");
			return 0;
		}

		int port = _wtoi(argv[1]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %s\n", argv[1]);
			return 1;
		}

		std::wstring pwd;
		char c;
		while ((c = _getch()) != '\n')
		{
			pwd += c;
			printf("*");
		}

		std::wstring legerFilePath = argv[2];
		leger = std::make_shared<securelib::lockerleger>(pwd, legerFilePath);

		printf("Setting up serving of port %d...", port);
		HttpServer httpServer(static_cast<uint16_t>(port), HandleRequests);
		httpServer.StartServing();
		printf("done!\n");

		while (true)
		{
#ifndef _DEBUG
			try
#endif
			{
				printf("\nquit, register <name>, checkin <name>, checkout <name>");
				printf("\n> ");
				std::wstring line;
				std::getline(std::wcin, line);
				if (line.empty())
					continue;

				if (line == L"quit")
					break;

				size_t spaceIndex = line.find(' ');
				if (spaceIndex == std::wstring::npos)
				{
					printf("Invalid command, no space\n");
					continue;
				}
				if (spaceIndex >= line.length() - 1)
				{
					printf("Invalid command, no name after space\n");
					continue;
				}

				std::wstring verb = line.substr(0, spaceIndex);
				std::wstring name = line.substr(spaceIndex + 1);

				if (verb == L"register")
				{
					leger->registerName(name);
					printf("\nClient registered.");
				}
				else if (verb == L"checkin")
				{
					uint32_t room;
					std::string key;
					leger->checkin(name, room, key);
					printf("\nClient checked in: Room: %d - Key: %s\n\n", 
						  int(room), key.c_str());
				}
				else if (verb == L"checkout")
				{
					leger->checkout(name);
					printf("\nClient checked out.");
				}
			}
#ifndef _DEBUG
			catch (const std::exception& exp)
			{
				printf("\nCommand ERROR: %s\n", exp.what());
				return 1;
			}
#endif
		}

		printf("\nShutting down...\n");
		httpServer.StopServing();
	}
#ifndef _DEBUG
	catch (const std::exception& exp)
	{
		printf("\nProgram ERROR: %s\n", exp.what());
		return 1;
	}
#endif
	printf("All done.\n");
	return 0;
}

Response HandleRequests(const Request& request)
{
	// FORNOW
	printf("%s %S\n", request.Verb.c_str(), Join(request.Path, L"/").c_str());
	Response response;
	return response;
}
