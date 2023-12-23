#include "Process.hpp"
#include <iostream>
#include "dbghelp.h"
#include <array>
#include <algorithm>
#include <string>
#include <fstream>

Process::Process(const char* window_name)
{
	HWND process_window = FindWindowA(nullptr, window_name);
	if (!process_window)
	{
		std::cerr << "Failed to get window" << std::endl;
		return;
	}
	GetWindowThreadProcessId(process_window, &pid);
	if (!pid)
	{
		std::cerr << "Failed to get process ID" << std::endl;
	}
	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

Process::~Process()
{
	if (process_handle)
		CloseHandle(process_handle);
}

Process::operator bool() const
{
	return process_handle != nullptr;
}

void Process::disableHooks()
{
	//std::string modules_to_check[] = {"jvm.dll" "ntdll.dll", "KERNEL32.DLL", "KERNELBASE.dll" };
	//char process_name[FILENAME_MAX + 1] = { 0 };
	//GetModuleBaseNameA(process_handle, nullptr, process_name, FILENAME_MAX);

	MODULEINFO info{};
	HMODULE modules[128] = { 0 };
	DWORD modules_size = 0;
	EnumProcessModulesEx(process_handle, modules, sizeof(modules), &modules_size, LIST_MODULES_64BIT);
	int module_count = modules_size / sizeof(HMODULE);

	for (int i = 0; i < module_count; ++i)
	{
		if (!modules[i]) continue;
		char module_base[FILENAME_MAX + 1] = { 0 };
		GetModuleBaseNameA(process_handle, modules[i], module_base, FILENAME_MAX);
		char module_path[MAX_PATH + 1] = { 0 };
		GetModuleFileNameExA(process_handle, modules[i], module_path, MAX_PATH);
		GetModuleInformation(process_handle, modules[i], &info, sizeof(MODULEINFO));
		//if (!std::strcmp(process_name, module_base)) continue;
		//if (std::find(std::begin(modules_to_check), std::end(modules_to_check), module_base) == std::end(modules_to_check)) continue;
		std::cout << module_base << '\n';
		//std::cout << module_path << '\n';

		uint8_t* module_base_addr = (uint8_t*)modules[i];
		uint8_t* module_end_addr = module_base_addr + info.SizeOfImage;
		std::unique_ptr<uint8_t[]> module_copy = read_memory(module_base_addr, info.SizeOfImage);

		
		PIMAGE_NT_HEADERS nt_header = ImageNtHeader(module_copy.get());
		int section_number = nt_header->FileHeader.NumberOfSections;
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);

		std::ifstream dllfile(module_path, std::ios::binary);
		if (!dllfile) continue;

		for (int s = 0; s < section_number; ++s, ++section)
		{
			if (std::strcmp((const char*)section->Name, ".text")) continue;

			uint8_t* start = module_copy.get() + section->VirtualAddress;

			std::string_view sv((char*)start, section->SizeOfRawData);
			std::vector<uint8_t*> jmps{};
			for (size_t it = sv.find('\xE9', 0); it != sv.npos; it = sv.find('\xE9', ++it))
			{
				uint8_t* found = start + it;
				uint32_t offset = found - module_copy.get();
				jmps.push_back(module_base_addr + offset);
			}

			for (uint8_t* jmp : jmps)
			{
				uint32_t offset = jmp - module_base_addr;
				uint8_t* possible_modified_bytes = module_copy.get() + offset;
				uint32_t virtual_section_offset = offset - section->VirtualAddress;
				uint32_t file_offset = section->PointerToRawData + virtual_section_offset;

				dllfile.seekg(file_offset, std::ios::beg);
				uint8_t read_bytes[5] = { 0 };
				dllfile.read((char*)read_bytes, 5);

				if (!memcmp(possible_modified_bytes, read_bytes, 5)) continue; //all is ok
				//Hook detected
				//try to restore original bytes
				write_memory(jmp, read_bytes, 5);
				std::cout << "Removed jmp at: " << (void*)jmp << "\n";
			}
		}
	}

	std::cout << "done" << '\n';
}