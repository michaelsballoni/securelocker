#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // toWideString / toNarrowString

#include "securelib.h"
#include "lockerleger.h"
#include "lockerfiles.h"
#include "lockerhttp.h"
#pragma comment(lib, "securelib")

#include "Core.h"
#include "HttpServer.h"
#pragma comment(lib, "httplite")

#include <atomic>
#include <conio.h> // _getch()
#include <filesystem>
#include <iostream>

using namespace httplite;
using namespace securelib;

namespace fs = std::filesystem;

int wmain(int argc, wchar_t* argv[])
{
	try
	{
		srand(time(nullptr) % RAND_MAX);

		if (argc != 3) {
			printf("Usage: <port> <locker root dir path>\n");
			return 0;
		}

		int port = _wtoi(argv[1]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %S\n", argv[1]);
			return 1;
		}

		printf("Leger Access Password: ");
		std::wstring password;
		while (true)
		{
			char c = _getch();
			if (c == '\r' || c == '\n')
				break;
			password += c;
			printf("*");
		}
		printf("\n");

		setLogFile(stdout);
#ifdef _DEBUG
		setLogTrace(true);
#endif
		SocketUse useSockets;

		std::wstring lockerRootDir = argv[2];
		std::wstring legerFilePath = fs::path(lockerRootDir).append("leger.dat");
		lockerleger leger(password, legerFilePath);
		leger.load();

		printf("Serving port %d...\n", port);
		lockerserver server
		(
			static_cast<uint16_t>(port), 
			leger, 
			lockerRootDir
		);
		server.start();

		printf("\nReady for your commands...\n");
		while (true)
		{
			try
			{
				printf("\nquit, register <name>, getroom <name>, checkin <name>, checkout <name>");
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
					leger.registerClient(name);
					printf("\nClient registered.\n");
				}
				else if (verb == L"getroom")
				{
					std::string room = leger.getRoom(name);
					printf("\nRoom: %s\n", room.c_str());
				}
				else if (verb == L"checkin")
				{
					uint32_t room;
					std::string key;
					leger.checkin(name, room, key);
					printf("\nClient checked in: Room: %d - Key: %s\n",
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
					printf("\nClient checked out.\n");
				}
				else
					throw std::runtime_error("Invalid command");
			}
			catch (const std::exception& exp)
			{
				printf("\nERROR: %s\n", exp.what());
			}
		}

		printf("\nShutting down...\n");
		server.stop();
	}
	catch (const std::exception& exp)
	{
		printf("\nSetup ERROR: %s\n", exp.what());
		return 1;
	}
	printf("All done.\n");
	return 0;
}
