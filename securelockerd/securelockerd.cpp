#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "lockerleger.h"
#include "lockerfiles.h"
#include "lockerhttp.h"
#include "securelib.h"

#include "Core.h"
#include "HttpServer.h"

#include <atomic>
#include <conio.h> // _getch()
#include <filesystem>
#include <iostream>

#pragma comment(lib, "httplite")
#pragma comment(lib, "securelib")

using namespace httplite;
using namespace securelib;

namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[])
{
#ifndef _DEBUG
	try
#endif
	{
		srand(time(nullptr) % RAND_MAX);
		std::atomic<int> lockerAuthNonce = rand();

		if (argc != 3) {
			printf("Usage: securelockerd <port> <leger file path>\n");
			return 0;
		}

		int port = _wtoi(argv[1]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %S\n", argv[1]);
			return 1;
		}

		std::wstring password;
		char c;
		while ((c = _getch()) != '\n')
		{
			password += c;
			printf("*");
		}

		std::wstring legerFilePath = argv[2];
		lockerleger leger(password, legerFilePath);
		std::wstring lockerRootDir = fs::path(legerFilePath).parent_path().wstring();

		printf("Setting up serving of port %d...", port);
		lockerserver server
		(
			static_cast<uint16_t>(port), 
			leger, 
			lockerRootDir, 
			lockerAuthNonce
		);
		server.start();
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
					leger.registerName(name);
					printf("\nClient registered.");
				}
				else if (verb == L"checkin")
				{
					uint32_t room;
					std::string key;
					leger.checkin(name, room, key);
					printf("\nClient checked in: Room: %d - Key: %s\n\n", 
						  int(room), key.c_str());
				}
				else if (verb == L"checkout")
				{
					uint32_t room = 0;
					leger.checkout(name, room);
					lockerfiles files(lockerRootDir, room);
					if (!files.dir().empty())
					{
						printf("Client files directory is not empty.  Proceed?  (y)es or (n)no: ");
						std::string response;
						std::getline(std::cin, response);
						if (response.empty() || toupper(response[0]) == 'N')
							continue;
					}
					files.checkout();
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
		server.stop();
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
