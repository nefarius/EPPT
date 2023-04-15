// WinAPI
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dwmapi.h>

// ImGui + SFML helper
#include "imgui.h"
#include "imgui-SFML.h"

// SFML
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "resource.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	auto vm = sf::VideoMode(280, 60);
	sf::RenderWindow window(vm, "Enhance Pointer Precision Tool", sf::Style::None);

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	// Enable window transparency
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(window.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
	DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);

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
					window.close();
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
		             | ImGuiWindowFlags_NoScrollbar);

		if (!isOpen) break;

		int mouseParams[3], accel;

		// Get the current values.
		SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);

		accel = mouseParams[2];

		// Modify the acceleration value as directed.
		ImGui::Checkbox("Enhance Pointer Precision enabled", reinterpret_cast<bool*>(&mouseParams[2]));

		// Value has been changed by user
		if (accel != mouseParams[2])
			// Update the system setting.
			SystemParametersInfo(SPI_SETMOUSE, 0, mouseParams, SPIF_SENDCHANGE);

		ImGui::End();

		window.clear(sf::Color::Transparent);

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
