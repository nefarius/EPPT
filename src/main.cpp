// WinAPI
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dwmapi.h>
#include <shellapi.h>

// ImGui + SFML helper
#include "imgui.h"
#include "imgui-SFML.h"

// SFML
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

//
// STL
// 
#include <format>

// CLI parser
#include <argh.h>

//
// Custom
// 
#include "UniUtil.h"
#include "resource.h"

//
// Enable visual styles
// 
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

static std::wstring GetImageBasePathW()
{
	wchar_t myPath[MAX_PATH + 1] = { 0 };

	GetModuleFileNameW(
		reinterpret_cast<HINSTANCE>(&__ImageBase),
		myPath,
		MAX_PATH + 1
	);

	return std::wstring(myPath);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(szCmdLine);
	UNREFERENCED_PARAMETER(iCmdShow);

#pragma region CLI parsing

	argh::parser cmdl;

	LPWSTR* szArglist;
	int nArgs;
	int i;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nullptr == szArglist)
	{
		return EXIT_FAILURE;
	}

	std::vector<const char*> argv;
	std::vector<std::string> narrow;

	for (i = 0; i < nArgs; i++)
	{
		narrow.push_back(ConvertWideToANSI(std::wstring(szArglist[i])));
	}

	argv.resize(nArgs);
	std::transform(narrow.begin(), narrow.end(), argv.begin(), [](const std::string& arg) { return arg.c_str(); });

	argv.push_back(nullptr);

	cmdl.parse(nArgs, &argv[0]);

#pragma endregion

#pragma region CLI processing

	//
	// Show help
	// 
	if (cmdl[{ "-h", "-?", "--help" }])
	{
		MessageBox(
			nullptr,
			LR"(The following command line switches are supported:

-h|-?|--help	Displays this help :) 
-d|--disable	Disable acceleration on launch
-e|--enable	Enable acceleration on launch
-a|--exit		Exit without creating a window

-r|--register-autostart	Self-registers this instance in current user's autostart
    --with-disable	Runs disable action on autostart
    --with-enable	Runs enable action on autostart
    --with-exit	Exits without window when done on autostart

-u|--unregister-autostart	Removes this instance from current user's autostart
)",
L"EPPT command line options",
MB_OK | MB_ICONINFORMATION
);

		return ERROR_SUCCESS;
	}

	int mParams[3];

	// Get the current values.
	SystemParametersInfo(SPI_GETMOUSE, 0, mParams, 0);

	if (cmdl[{ "-d", "--disable" }])
	{
		mParams[2] = false;
	}
	else if (cmdl[{ "-e", "--enable" }])
	{
		mParams[2] = true;
	}

	// Update the system setting.
	SystemParametersInfo(SPI_SETMOUSE, 0, mParams, SPIF_SENDCHANGE);

	// register ourselves in autostart of current user
	if (cmdl[{ "-r", "--register-autostart" }])
	{
		HKEY hKey = nullptr;
		std::wstring myself = GetImageBasePathW();
		RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);

		bool withDisable = cmdl[{ "--with-disable" }];
		bool withEnable = cmdl[{ "--with-enable" }];
		bool withExit = cmdl[{ "--with-exit" }];

		std::wstring command = std::format(
			L"\"{}\" {} {} {}",
			myself,
			withDisable ? L"-d" : L"",
			withEnable ? L"-e" : L"",
			withExit ? L"-a" : L""
		);

		RegSetValueEx(
			hKey,
			L"EPPT",
			0,
			REG_SZ,
			reinterpret_cast<BYTE*>(command.data()),
			static_cast<DWORD>(wcslen(command.c_str()) + 1) * 2
		);
	}

	// remove ourselves from autostart of current user
	if (cmdl[{ "-u", "--unregister-autostart" }])
	{
		HKEY hKey = nullptr;
		RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
		RegDeleteValue(hKey, L"EPPT");
	}

	// do not create window, exit if instructed
	if (cmdl[{ "-a", "--exit" }])
	{
		return ERROR_SUCCESS;
	}

#pragma endregion

	LPCWSTR settingsName = L"WindowPlacement";
	auto vm = sf::VideoMode(280, 60);
	sf::RenderWindow window(vm, "Enhance Pointer Precision Tool", sf::Style::None);

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	// Enable window transparency
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(window.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
	DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);

	WINDOWPLACEMENT placement;
	DWORD valueType, bytesRead = sizeof(WINDOWPLACEMENT);
	HKEY appKey = NULL;

	// create/query settings key
	LSTATUS status = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\Nefarius Software Solutions e.U.\\EPPT",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&appKey,
		NULL
	);

	if (status == ERROR_SUCCESS)
	{
		status = RegQueryValueEx(
			appKey,
			settingsName,
			0,
			&valueType,
			(LPBYTE)&placement,
			&bytesRead
		);

		// restore last window position
		if (status == ERROR_SUCCESS && bytesRead == sizeof(WINDOWPLACEMENT))
		{
			SetWindowPlacement(window.getSystemHandle(), &placement);
		}
	}

	// set topmost flag
	SetWindowPos(window.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Set window icon
	auto hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	if (hIcon)
	{
		SendMessage(window.getSystemHandle(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	}

	sf::Vector2i grabbedOffset;
	auto grabbedWindow = false;
	auto isOpen = true;
	window.resetGLStates();
	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			// Escape key closes window/application
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}
			}
			// Mouse events used to react to dragging
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					grabbedOffset = window.getPosition() - sf::Mouse::getPosition();
					grabbedWindow = true;
				}
			}
			// Mouse events used to react to dragging
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
					grabbedWindow = false;
			}
			// Mouse events used to react to dragging
			else if (event.type == sf::Event::MouseMoved)
			{
				if (grabbedWindow)
					window.setPosition(sf::Mouse::getPosition() + grabbedOffset);
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		// Create main window
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2((vm.width * 1.0f), (vm.height * 1.0f)));
		ImGui::Begin("Enhance Pointer Precision Tool", &isOpen,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoScrollbar
		);

		if (!isOpen) break;

#pragma region Change acceleration settings

		int mouseParams[3], accel;

		// Get the current values.
		SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);

		accel = mouseParams[2];

		// Modify the acceleration value as directed.
		ImGui::Checkbox("Enhance Pointer Precision enabled", reinterpret_cast<bool*>(&mouseParams[2]));

		// Value has been changed by user
		if (accel != mouseParams[2])
		{
			// Update the system setting.
			SystemParametersInfo(SPI_SETMOUSE, 0, mouseParams, SPIF_SENDCHANGE);
		}

#pragma endregion

		ImGui::End();

		window.clear(sf::Color::Transparent);

		ImGui::SFML::Render(window);
		window.display();
	}

	// store window position in registry
	if (appKey)
	{
		GetWindowPlacement(window.getSystemHandle(), &placement);

		RegSetValueEx(
			appKey,
			settingsName,
			0,
			REG_BINARY,
			(LPBYTE)&placement,
			sizeof(WINDOWPLACEMENT)
		);

		RegCloseKey(appKey);
	}

	ImGui::SFML::Shutdown();

	return 0;
}
