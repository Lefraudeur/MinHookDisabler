#include <Windows.h>
#include <memory>
#include <Psapi.h>
#include <vector>
#include <cstdint>


class Process
{
public:
	Process(const char* window_name);
	~Process();

	operator bool() const;

	void disableHooks();

	template<typename T> inline std::unique_ptr<T[]> read_memory(T* address, int element_count)
	{
		if (!process_handle)
			return std::unique_ptr<T[]>{};
		std::unique_ptr<T[]> data = std::make_unique<T[]>(element_count);
		ReadProcessMemory(process_handle, address, data.get(), element_count * sizeof(T), nullptr);
		return data;
	}

	template<typename T> inline T read_memory(T* address)
	{
		if (!process_handle)
			return T();
		T buffer{};
		ReadProcessMemory(process_handle, address, &buffer, sizeof(T), nullptr);
		return buffer;
	}

	template<typename T> inline size_t write_memory(T* address, T& data)
	{
		if (!process_handle)
			return 0;
		size_t bytes_written = 0;
		WriteProcessMemory(process_handle, address, &data, sizeof(T), &bytes_written);
		return bytes_written;
	}

	template<typename T> inline size_t write_memory(T* address, T array[], int element_count)
	{
		if (!process_handle)
			return 0;
		size_t bytes_written = 0;
		WriteProcessMemory(process_handle, address, array, sizeof(T) * element_count, &bytes_written);
		return bytes_written;
	}
private:
	HANDLE process_handle = nullptr;
	DWORD pid = 0;
};
