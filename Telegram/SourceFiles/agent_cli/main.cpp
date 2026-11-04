/*
This file is part of WickedGram,
an open-source fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

#include <windows.h>
#include <winhttp.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#pragma comment(lib, "winhttp.lib")

namespace {

std::wstring ToWString(const std::string &str) {
	if (str.empty()) {
		return std::wstring();
	}
	const int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), int(str.size()), nullptr, 0);
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), int(str.size()), &result[0], size);
	return result;
}

std::string ToString(const std::wstring &wstr) {
	if (wstr.empty()) {
		return std::string();
	}
	const int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), int(wstr.size()), nullptr, 0, nullptr, nullptr);
	std::string result(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), int(wstr.size()), &result[0], size, nullptr, nullptr);
	return result;
}

std::string EscapeJsonString(const std::string &src) {
	std::string res;
	res.reserve(src.size() + 16);
	for (char c : src) {
		switch (c) {
		case '\"': res += "\\\""; break;
		case '\\': res += "\\\\"; break;
		case '\b': res += "\\b"; break;
		case '\f': res += "\\f"; break;
		case '\n': res += "\\n"; break;
		case '\r': res += "\\r"; break;
		case '\t': res += "\\t"; break;
		default: res += c; break;
		}
	}
	return res;
}

std::string UrlEncode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;
	for (unsigned char c : value) {
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
		} else {
			escaped << '%' << (int)c;
		}
	}
	return escaped.str();
}

struct Config {
	int port = 40404;
	std::string token;
};

std::string ReadFileContent(const std::wstring &path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return "";
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

Config DiscoverConfig() {
	Config config;

	// 1. Check environment variables
	char *envPort = nullptr;
	size_t len = 0;
	if (_dupenv_s(&envPort, &len, "WICKEDGRAM_AGENT_PORT") == 0 && envPort) {
		config.port = std::atoi(envPort);
		free(envPort);
	}
	char *envToken = nullptr;
	if (_dupenv_s(&envToken, &len, "WICKEDGRAM_AGENT_TOKEN") == 0 && envToken) {
		config.token = envToken;
		free(envToken);
	}

	if (!config.token.empty()) {
		return config;
	}

	// 2. Candidate configuration files
	std::vector<std::wstring> candidates = {
		L"tdata/wickedgram_agent.json",
		L"../tdata/wickedgram_agent.json",
		L"D:/WickedGram-dev/tdata/wickedgram_agent.json",
		L"../WickedGram-dev/tdata/wickedgram_agent.json",
		L"out/Debug/tdata/wickedgram_agent.json",
		L"out/Release/tdata/wickedgram_agent.json"
	};

	wchar_t appdata[MAX_PATH] = { 0 };
	if (GetEnvironmentVariableW(L"APPDATA", appdata, MAX_PATH) > 0) {
		candidates.push_back(std::wstring(appdata) + L"/Telegram Desktop/tdata/wickedgram_agent.json");
	}

	for (const auto &path : candidates) {
		const auto content = ReadFileContent(path);
		if (content.empty()) {
			continue;
		}
		// Extract token
		const auto tokenKey = content.find("\"token\"");
		if (tokenKey != std::string::npos) {
			const auto colon = content.find(':', tokenKey);
			if (colon != std::string::npos) {
				const auto quote1 = content.find('\"', colon);
				if (quote1 != std::string::npos) {
					const auto quote2 = content.find('\"', quote1 + 1);
					if (quote2 != std::string::npos) {
						config.token = content.substr(quote1 + 1, quote2 - quote1 - 1);
					}
				}
			}
		}
		// Extract port
		const auto portKey = content.find("\"port\"");
		if (portKey != std::string::npos) {
			const auto colon = content.find(':', portKey);
			if (colon != std::string::npos) {
				int val = 0;
				if (sscanf_s(content.c_str() + colon + 1, "%d", &val) == 1 && val > 0) {
					config.port = val;
				}
			}
		}
		if (!config.token.empty()) {
			break;
		}
	}

	return config;
}

struct HttpResponse {
	int statusCode = 0;
	std::string body;
	std::string error;
};

HttpResponse HttpRequest(
		const std::string &method,
		const std::string &path,
		int port,
		const std::string &token,
		const std::string &bodyJson = "") {
	HttpResponse response;

	HINTERNET hSession = WinHttpOpen(
		L"WickedGram-Agent-CLI/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (!hSession) {
		response.error = "WinHttpOpen failed: " + std::to_string(GetLastError());
		return response;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, L"127.0.0.1", port, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		response.error = "Failed to connect to 127.0.0.1:" + std::to_string(port);
		return response;
	}

	const auto methodW = ToWString(method);
	const auto pathW = ToWString(path);

	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		methodW.c_str(),
		pathW.c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		response.error = "WinHttpOpenRequest failed";
		return response;
	}

	std::wstring headers = L"Accept: application/json\r\n";
	if (!token.empty()) {
		headers += L"Authorization: Bearer " + ToWString(token) + L"\r\n";
	}
	if (!bodyJson.empty()) {
		headers += L"Content-Type: application/json; charset=utf-8\r\n";
	}

	const BOOL sent = WinHttpSendRequest(
		hRequest,
		headers.c_str(),
		DWORD(headers.size()),
		bodyJson.empty() ? nullptr : (LPVOID)bodyJson.data(),
		DWORD(bodyJson.size()),
		DWORD(bodyJson.size()),
		0);

	if (!sent || !WinHttpReceiveResponse(hRequest, nullptr)) {
		const auto err = GetLastError();
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		response.error = "Failed to communicate with WickedGram Agent Bridge on port "
			+ std::to_string(port) + " (Error code: " + std::to_string(err) + "). Is WickedGram running?";
		return response;
	}

	DWORD statusCode = 0;
	DWORD size = sizeof(statusCode);
	WinHttpQueryHeaders(
		hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode,
		&size,
		WINHTTP_NO_HEADER_INDEX);
	response.statusCode = int(statusCode);

	DWORD bytesAvailable = 0;
	while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
		std::vector<char> buffer(bytesAvailable);
		DWORD bytesRead = 0;
		if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead) && bytesRead > 0) {
			response.body.append(buffer.data(), bytesRead);
		}
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return response;
}

void ListenEvents(int port, const std::string &token, const std::string &filterChat) {
	HINTERNET hSession = WinHttpOpen(
		L"WickedGram-Agent-CLI/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (!hSession) {
		std::cerr << "WinHttpOpen failed" << std::endl;
		return;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, L"127.0.0.1", port, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		std::cerr << "Failed to connect to 127.0.0.1:" << port << std::endl;
		return;
	}

	const std::wstring path = L"/api/events?token=" + ToWString(UrlEncode(token));
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		L"GET",
		path.c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);

	std::wstring headers = L"Accept: text/event-stream\r\nAuthorization: Bearer " + ToWString(token) + L"\r\n";
	if (!WinHttpSendRequest(hRequest, headers.c_str(), DWORD(headers.size()), nullptr, 0, 0, 0)
		|| !WinHttpReceiveResponse(hRequest, nullptr)) {
		std::cerr << "Failed to start SSE event listener" << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	std::cout << "[*] Listening for real-time events on 127.0.0.1:" << port << " (Press Ctrl+C to stop)..." << std::endl;

	std::string lineBuf;
	DWORD bytesAvailable = 0;
	while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
		std::vector<char> buffer(bytesAvailable);
		DWORD bytesRead = 0;
		if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead) && bytesRead > 0) {
			for (DWORD i = 0; i < bytesRead; ++i) {
				char c = buffer[i];
				if (c == '\n') {
					if (!lineBuf.empty() && lineBuf.back() == '\r') {
						lineBuf.pop_back();
					}
					if (!lineBuf.empty()) {
						if (!filterChat.empty()) {
							if (lineBuf.find("\"chat_id\":\"" + filterChat + "\"") != std::string::npos
								|| lineBuf.find("\"chat_id\":" + filterChat) != std::string::npos) {
								std::cout << lineBuf << std::endl;
							}
						} else {
							std::cout << lineBuf << std::endl;
						}
					}
					lineBuf.clear();
				} else {
					lineBuf += c;
				}
			}
		}
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
}

void PrintHelp() {
	std::cout << "WickedGram Native AI Agent CLI\n"
		<< "Zero-dependency native controller for WickedGram Desktop\n\n"
		<< "Usage: agent [options] <command> [arguments]\n\n"
		<< "Commands:\n"
		<< "  status                           Check bridge and user status\n"
		<< "  chats list [--limit N] [--query Q]  List recent chats\n"
		<< "  chats get <chat_id>              Get details of a chat\n"
		<< "  messages get <chat_id> [--limit N]  Read messages from chat\n"
		<< "  messages send <chat_id> <text> [--reply-to ID]  Send message or reply\n"
		<< "  messages read <chat_id>          Mark chat as read\n"
		<< "  typing <chat_id>                 Send typing status indicator\n"
		<< "  ui open <chat_id>                Focus and open chat in WickedGram window\n"
		<< "  regdate <user_id>                Calculate estimated account registration date\n"
		<< "  backup export                    Export settings to JSON backup\n"
		<< "  backup import [--file <path>]    Import settings from backup file\n"
		<< "  events listen [--filter-chat ID] Listen to real-time events stream (SSE)\n\n"
		<< "Options:\n"
		<< "  --port <port>                    Override bridge port (default: auto)\n"
		<< "  --token <token>                  Override Bearer token (default: auto)\n"
		<< "  --json                           Output structured JSON (default: true)\n"
		<< "  -h, --help                       Show this help message\n";
}

} // namespace

int main(int argc, char *argv[]) {
	SetConsoleOutputCP(CP_UTF8);

	Config config = DiscoverConfig();

	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--port" && i + 1 < argc) {
			config.port = std::atoi(argv[++i]);
		} else if (arg == "--token" && i + 1 < argc) {
			config.token = argv[++i];
		} else if (arg == "--help" || arg == "-h") {
			PrintHelp();
			return 0;
		} else if (arg == "--json") {
			// json flag accepted
		} else {
			args.push_back(arg);
		}
	}

	if (args.empty()) {
		PrintHelp();
		return 1;
	}

	const auto cmd = args[0];

	if (cmd == "status") {
		const auto res = HttpRequest("GET", "/api/status", config.port, config.token);
		if (!res.error.empty()) {
			std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
			return 1;
		}
		std::cout << res.body << std::endl;
		return (res.statusCode == 200) ? 0 : 1;
	} else if (cmd == "chats") {
		if (args.size() < 2) {
			std::cerr << "{\"status\":\"error\",\"error\":\"Missing chats subcommand (list|get)\"}" << std::endl;
			return 1;
		}
		const auto sub = args[1];
		if (sub == "list") {
			int limit = 50;
			std::string query;
			for (size_t i = 2; i < args.size(); ++i) {
				if (args[i] == "--limit" && i + 1 < args.size()) {
					limit = std::atoi(args[++i].c_str());
				} else if (args[i] == "--query" && i + 1 < args.size()) {
					query = args[++i];
				}
			}
			std::string path = "/api/chats?limit=" + std::to_string(limit);
			if (!query.empty()) {
				path += "&query=" + UrlEncode(query);
			}
			const auto res = HttpRequest("GET", path, config.port, config.token);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		} else if (sub == "get") {
			if (args.size() < 3) {
				std::cerr << "{\"status\":\"error\",\"error\":\"Usage: chats get <chat_id>\"}" << std::endl;
				return 1;
			}
			const auto res = HttpRequest("GET", "/api/chats/" + args[2], config.port, config.token);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		}
	} else if (cmd == "messages") {
		if (args.size() < 2) {
			std::cerr << "{\"status\":\"error\",\"error\":\"Missing messages subcommand (get|send|read)\"}" << std::endl;
			return 1;
		}
		const auto sub = args[1];
		if (sub == "get") {
			if (args.size() < 3) {
				std::cerr << "{\"status\":\"error\",\"error\":\"Usage: messages get <chat_id> [--limit N]\"}" << std::endl;
				return 1;
			}
			int limit = 30;
			if (args.size() >= 5 && args[3] == "--limit") {
				limit = std::atoi(args[4].c_str());
			}
			const auto res = HttpRequest("GET", "/api/chats/" + args[2] + "/messages?limit=" + std::to_string(limit), config.port, config.token);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		} else if (sub == "send") {
			if (args.size() < 4) {
				std::cerr << "{\"status\":\"error\",\"error\":\"Usage: messages send <chat_id> <text> [--reply-to ID]\"}" << std::endl;
				return 1;
			}
			const auto chatId = args[2];
			const auto text = args[3];
			long long replyTo = 0;
			for (size_t i = 4; i < args.size(); ++i) {
				if (args[i] == "--reply-to" && i + 1 < args.size()) {
					replyTo = std::atoll(args[++i].c_str());
				}
			}
			std::string body = "{\"text\":\"" + EscapeJsonString(text) + "\"";
			if (replyTo != 0) {
				body += ",\"reply_to\":" + std::to_string(replyTo);
			}
			body += "}";

			const auto res = HttpRequest("POST", "/api/chats/" + chatId + "/messages", config.port, config.token, body);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		} else if (sub == "read") {
			if (args.size() < 3) {
				std::cerr << "{\"status\":\"error\",\"error\":\"Usage: messages read <chat_id>\"}" << std::endl;
				return 1;
			}
			const auto res = HttpRequest("POST", "/api/chats/" + args[2] + "/read", config.port, config.token);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		}
	} else if (cmd == "typing") {
		if (args.size() < 2) {
			std::cerr << "{\"status\":\"error\",\"error\":\"Usage: typing <chat_id>\"}" << std::endl;
			return 1;
		}
		const auto res = HttpRequest("POST", "/api/chats/" + args[1] + "/typing", config.port, config.token);
		if (!res.error.empty()) {
			std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
			return 1;
		}
		std::cout << res.body << std::endl;
		return (res.statusCode == 200) ? 0 : 1;
	} else if (cmd == "ui") {
		if (args.size() < 3 || args[1] != "open") {
			std::cerr << "{\"status\":\"error\",\"error\":\"Usage: ui open <chat_id>\"}" << std::endl;
			return 1;
		}
		const std::string body = "{\"chat_id\":\"" + EscapeJsonString(args[2]) + "\"}";
		const auto res = HttpRequest("POST", "/api/ui/open_chat", config.port, config.token, body);
		if (!res.error.empty()) {
			std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
			return 1;
		}
		std::cout << res.body << std::endl;
		return (res.statusCode == 200) ? 0 : 1;
	} else if (cmd == "regdate") {
		if (args.size() < 2) {
			std::cerr << "{\"status\":\"error\",\"error\":\"Usage: regdate <user_id>\"}" << std::endl;
			return 1;
		}
		const auto res = HttpRequest("GET", "/api/user/regdate?user_id=" + args[1], config.port, config.token);
		if (!res.error.empty()) {
			std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
			return 1;
		}
		std::cout << res.body << std::endl;
		return (res.statusCode == 200) ? 0 : 1;
	} else if (cmd == "backup") {
		if (args.size() < 2) {
			std::cerr << "{\"status\":\"error\",\"error\":\"Usage: backup (export|import)\"}" << std::endl;
			return 1;
		}
		const auto sub = args[1];
		if (sub == "export") {
			const auto res = HttpRequest("POST", "/api/settings/backup/export", config.port, config.token);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		} else if (sub == "import") {
			std::string filePath;
			if (args.size() >= 4 && args[2] == "--file") {
				filePath = args[3];
			}
			std::string body = "{}";
			if (!filePath.empty()) {
				body = "{\"file_path\":\"" + EscapeJsonString(filePath) + "\"}";
			}
			const auto res = HttpRequest("POST", "/api/settings/backup/import", config.port, config.token, body);
			if (!res.error.empty()) {
				std::cerr << "{\"status\":\"error\",\"error\":\"" << EscapeJsonString(res.error) << "\"}" << std::endl;
				return 1;
			}
			std::cout << res.body << std::endl;
			return (res.statusCode == 200) ? 0 : 1;
		}
	} else if (cmd == "events") {
		if (args.size() < 2 || args[1] != "listen") {
			std::cerr << "{\"status\":\"error\",\"error\":\"Usage: events listen [--filter-chat ID]\"}" << std::endl;
			return 1;
		}
		std::string filterChat;
		if (args.size() >= 4 && args[2] == "--filter-chat") {
			filterChat = args[3];
		}
		ListenEvents(config.port, config.token, filterChat);
		return 0;
	} else {
		std::cerr << "{\"status\":\"error\",\"error\":\"Unknown command: " << cmd << "\"}" << std::endl;
		return 1;
	}

	return 0;
}
