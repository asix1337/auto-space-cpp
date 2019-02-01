/*
* MIT License
*
* Copyright (c) 2019 3N7R0PY1337
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
**/

#include <chrono>
#include <thread>
#include <iostream>
#include <random>

#include <Windows.h>
#include <VersionHelpers.h>

#include "resource.h"

using namespace std::literals::chrono_literals;

auto WINAPI ThreadProc(LPVOID)->DWORD;
auto CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM)->LRESULT;

BOOL bIsEnabled { FALSE };
HANDLE hThread { nullptr };
HHOOK hHook { nullptr };

auto wmain(int, wchar_t*[]) -> int
{
	{
		SetConsoleTitleW(L"Auto Space");
		((BOOL(WINAPI*)(HICON))GetProcAddress(GetModuleHandleW(L"kernel32"),
			"SetConsoleIcon"))(LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_ICON1)));

		HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		if (IsWindowsVistaOrGreater())
		{
			CONSOLE_FONT_INFOEX cfi { 0 };
			GetCurrentConsoleFontEx(hConsoleOutput, FALSE, &cfi);
			cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
			cfi.nFont = 9UL;
			cfi.dwFontSize.X = 10;
			cfi.dwFontSize.Y = 16;
			cfi.FontFamily = 54U;
			cfi.FontWeight = 400U;
			wcscpy_s(cfi.FaceName, L"Consolas");
			SetCurrentConsoleFontEx(hConsoleOutput, FALSE, &cfi);
		}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           		SetConsoleScreenBufferSize(hConsoleOutput, { 100, 30 });
		SMALL_RECT cw { 0, 0, 100 - 1, 30 - 1 };
		SetConsoleWindowInfo(hConsoleOutput, TRUE, &cw);

		HWND hWnd = GetConsoleWindow();
		int nWidth = GetSystemMetrics(SM_CXSCREEN), nHeight = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hWnd, (nWidth / 4), (nHeight / 4), (nWidth / 2), (nHeight / 2), TRUE);
		SetWindowLongW(hWnd, GWL_EXSTYLE, GetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, 0UL, 220, LWA_ALPHA);
		SetConsoleTextAttribute(hConsoleOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}

	hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0UL);

	std::wcout << L"Press \"DEL\" key to start. Press \"DEL\" key again to stop. Press CTRL+C to Exit." << std::endl;

	SetConsoleCtrlHandler([] (DWORD dwCtrlType) -> BOOL
	{
		if (dwCtrlType == CTRL_C_EVENT)
			std::exit(EXIT_SUCCESS);
		return TRUE;
	}, TRUE);

	MSG msg;
	while (!GetMessageW(&msg, nullptr, 0U, 0U))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	UnhookWindowsHookEx(hHook);

	return EXIT_SUCCESS;
}

auto WINAPI ThreadProc(LPVOID lpParameter) -> DWORD
{
	INPUT input;
	ZeroMemory(&input, sizeof(input));
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = VK_SPACE;
	input.ki.wScan = MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC);

	std::mt19937 mt(std::random_device {}());
	std::uniform_int_distribution<int> dist { 10, 25 };
	auto const interval { std::chrono::milliseconds{dist(mt)} };
	std::wcout << L"Started with an interval of " << interval.count() << " milliseconds." << std::endl;

	while (bIsEnabled)
	{
		input.ki.dwFlags = 0UL;
		SendInput(1U, &input, sizeof(INPUT));
		//keybd_event(VK_SPACE, MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC), 0, 0);
		std::this_thread::sleep_for(interval);
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1U, &input, sizeof(INPUT));
		//keybd_event(VK_SPACE, MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
	}
	std::wcout << L"Stopped." << std::endl;
	return 0;
}

auto CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	if (wParam == WM_KEYUP && reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam)->vkCode == VK_DELETE)
	{
		if (!bIsEnabled && hThread == nullptr)
		{
			bIsEnabled = TRUE;
			hThread = CreateThread(nullptr, 0U, ThreadProc, nullptr, 0UL, nullptr);
		}
		else
		{
			bIsEnabled = FALSE;
			WaitForSingleObject(hThread, INFINITE);
			Beep(1000, 100);
			hThread = nullptr;
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
