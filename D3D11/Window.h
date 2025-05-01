#pragma once
#include <Windows.h>
class Window
{

private:
	class WindowClass {
	public:
		static const wchar_t* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		static constexpr const wchar_t* wndClassName = L"Agni Reload Window Class";
		HINSTANCE hInst;
		static WindowClass wndClass;
	};
public:
	Window(const wchar_t* windowName, UINT width, UINT height);
	~Window();
	Window(const Window&) = delete;
	Window operator = (const Window&) = delete;
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM LParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM LParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)noexcept;

private:
	HWND hwnd;
};

