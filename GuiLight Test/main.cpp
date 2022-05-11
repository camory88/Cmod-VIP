#include <iostream>
#include <thread>
#include "overlay.h"

int main()
{
	process_id = get_process_id("r5apex.exe");//Getting the Process Id
	base_address = get_module_base_address("r5apex.exe");//Getting The Module Base

	cout << "Process id found at " << process_id << endl;//Printing the Process Id (For Debugging and check if all ok)
	cout << "Process Base Address found at 0x" << hex << base_address << endl;

	if (!overlay::InitWindow()) { printf("Failed to create window\n"); Sleep(5000); return 0; }
	if (!overlay::DirectXInit()) { printf("Failed to initialize directx\n"); Sleep(5000); return 0; }
	std::thread(overlay::Render).detach();
	while (true)
	{
		
		Sleep(100);
	}
	overlay::DirectXShutdown();
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
