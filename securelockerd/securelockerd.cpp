#include "Leger.h"

#include "HttpServer.h"

#include <conio.h>

#include <iostream>

using namespace httplite;

Response HandleRequests(const Request& request);

securelocker::Leger leger;

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2) {
			printf("Usage: securelockerd <port>\n");
			return 0;
		}

		std::wstring pwd;
		char c;
		while ((c = getch()) != '\n')
			pwd += c;

		leger.load(pwd);


		int port = atoi(argv[1]);
		if (port <= 0 || port > USHRT_MAX) {
			printf("ERROR: Invalid port number: %s\n", argv[1]);
			return 1;
		}

		printf("Setting up serving of port %d...", port);;
		HttpServer httpServer(static_cast<uint16_t>(port), HandleRequests);
		httpServer.StartServing();
		printf("done!\n");

		while (true)
		{
			printf("\nquit, reserve <name>, checkin <name>, checkout <name>");
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
			std::wstring noun = line.substr(spaceIndex + 1);

			if (verb == L"checkin")
			{
				// FORNOW
				// take name
				// leger.createEntry
				// print room # and key to console for giving to client out of band
			}
			else if (verb == L"checkout")
			{
				// FORNOW
				// take name and room #
				// leger.removeEntry
			}
		}

		printf("Shutting down...\n");
		httpServer.StopServing();

	}
	catch (const std::exception& exp)
	{
		printf("\nERROR: %s\n", exp.what());
		return 1;
	}

	printf("All done.\n");
	return 0;
}

Response HandleRequests(const Request& request)
{
	// FORNOW
	printf("%s %S\n", request.Path, Join(request.Path, L"/"));
}
