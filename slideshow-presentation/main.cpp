#include <Windows.h>
#include <gl/GL.h>
#include "utils/opengl/opengl_defines.h"
#include "utils/opengl/opengl_funcs.h"


static void fatal_error(const char* msg) {
	MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
	exit(EXIT_FAILURE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASS windowClass = {};

	windowClass.lpfnWndProc = WindowProc;
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"Class name";

	if (!RegisterClass(&windowClass)) {
		fatal_error("Failed to register OpenGL window");
	}

	HWND fake_window = CreateWindow(
		L"Class name", L"Fake Window",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,
		1, 1,
		NULL, NULL,
		hInstance, NULL);

	HDC fake_dc = GetDC(fake_window);

	PIXELFORMATDESCRIPTOR fake_pfd;
	ZeroMemory(&fake_pfd, sizeof(fake_pfd));
	fake_pfd.nSize = sizeof(fake_pfd);
	fake_pfd.nVersion = 1;
	fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fake_pfd.iPixelType = PFD_TYPE_RGBA;
	fake_pfd.cColorBits = 32;
	fake_pfd.cAlphaBits = 8;
	fake_pfd.cDepthBits = 24;


	int fake_pfd_id = ChoosePixelFormat(fake_dc, &fake_pfd);
	if (fake_pfd_id == 0) {
		fatal_error("ChoosePixelFormat failed");
	}

	if (SetPixelFormat(fake_dc, fake_pfd_id, &fake_pfd) == false) {
		fatal_error("SetPixelFormat failed");
	}

	HGLRC fake_rc = wglCreateContext(fake_dc);

	if (fake_rc == 0) {
		fatal_error("wglCreateContext failed");
	}

	if (wglMakeCurrent(fake_dc, fake_rc) == false)
	{
		fatal_error("wglMakeCurrent failed");
	}


	wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress("wglCreateContextAttribsARB");

	if (wglChoosePixelFormatARB == nullptr)
	{
		fatal_error("wglGetProcAddress failed");
	}

	if (wglCreateContextAttribsARB == nullptr)
	{
		fatal_error("wglGetProcAdress failed");
	}


	HWND window = CreateWindow(
		windowClass.lpszClassName,
		L"SlideShow",
		WS_OVERLAPPEDWINDOW,
		200,
		200,
		1280,
		720,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!window) {
		fatal_error("Failed to create window");
	}

	HDC dc = GetDC(window);

	const int pixelAttribs[] = {
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_ALPHA_BITS_ARB, 8,
	WGL_DEPTH_BITS_ARB, 24,
	WGL_STENCIL_BITS_ARB, 8,
	WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
	WGL_SAMPLES_ARB, 4,
	0
	};

	int pixel_format_id;
	UINT num_formats;

	bool status = wglChoosePixelFormatARB(dc, pixelAttribs, NULL, 1, &pixel_format_id, &num_formats);

	if (status == false || num_formats == 0) {
		fatal_error("wglChoosePixelFormatARB failed");
	}

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(dc, pixel_format_id, sizeof(pfd), &pfd);
	SetPixelFormat(dc, pixel_format_id, &pfd);

	const int major_min = 4, minor_min = 5;
	int  contextAttribs[] = {
	WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
	WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0
	};
	HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
	if (rc == NULL) {
		fatal_error("wglCreateContextAttribsARB failed");
	}
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_rc);
	ReleaseDC(fake_window, fake_dc);
	//DestroyWindow(fake_window);
	if (!wglMakeCurrent(dc, rc)) {
		fatal_error("wglMakeCurrent failed");
	}

	ShowWindow(window, nShowCmd);

	bool running = true;

	while (running) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				running = false;
			}
			else {
				TranslateMessage(&message);
				DispatchMessageA(&message);
			}
		}

		glClearColor(0.129f, 0.586f, 0.949f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SwapBuffers(dc);
	}

	return 0;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
	
	switch (wMsg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	return 0;
	}
	return DefWindowProc(hwnd, wMsg, wParam, lParam);
}