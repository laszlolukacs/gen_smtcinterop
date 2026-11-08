/* See LICENSE for copyright details. */

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cwchar>

/**
 * Appends one line to "%TEMP%\gen_smtcinterop.log".
 *
 * This is intentionally tiny and dependency-free so it can be called from any
 * thread (window procedure, SMTC event callback, etc.) while diagnosing
 * multimedia key handling.
 */
inline void debug_log(const wchar_t* format, ...)
{
	wchar_t temp_path[MAX_PATH];
	const DWORD temp_path_length = GetTempPathW(MAX_PATH, temp_path);
	if (temp_path_length == 0 || temp_path_length >= MAX_PATH)
	{
		return;
	}

	wchar_t log_path[MAX_PATH];
	if (swprintf_s(log_path, L"%sgen_smtcinterop.log", temp_path) < 0)
	{
		return;
	}

	SYSTEMTIME now;
	GetLocalTime(&now);

	wchar_t message[2048];
	const int prefix_length = swprintf_s(
		message,
		L"[%02u:%02u:%02u.%03u tid=%lu] ",
		now.wHour,
		now.wMinute,
		now.wSecond,
		now.wMilliseconds,
		GetCurrentThreadId());
	if (prefix_length < 0)
	{
		return;
	}

	va_list arguments;
	va_start(arguments, format);
	const int written = vswprintf_s(
		message + prefix_length,
		_countof(message) - prefix_length,
		format,
		arguments);
	va_end(arguments);
	if (written < 0)
	{
		return;
	}

	size_t message_length = (size_t)prefix_length + (size_t)written;
	message[message_length++] = L'\r';
	message[message_length++] = L'\n';
	message[message_length] = L'\0';

	char utf8[4096];
	const int utf8_length = WideCharToMultiByte(
		CP_UTF8,
		0,
		message,
		(int)message_length,
		utf8,
		(int)sizeof(utf8),
		nullptr,
		nullptr);
	if (utf8_length <= 0)
	{
		return;
	}

	HANDLE file = CreateFileW(
		log_path,
		FILE_APPEND_DATA,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD bytes_written = 0;
	WriteFile(file, utf8, (DWORD)utf8_length, &bytes_written, nullptr);
	CloseHandle(file);
}

#endif // DEBUG_LOG_H
