/*
	Virtual method table. You can find more information here: https://en.wikipedia.org/wiki/Virtual_method_table
*/
#include "misc.hpp"

class entity_t
{
private:
	int x = 15, y = 25;

public:
	virtual void print(std::string name) { std::cout << "Entity: " << name << std::endl; }
	virtual std::string get_name() { return "Unknown"; }
};

class player_t : public entity_t
{
public:
	void print(std::string name) override { std::cout << "Idiot: " << name << std::endl; }
	std::string get_name() override { return "Gosho"; }
};

void print_vtable(entity_t* entity)
{
	const auto vtable_ptr = *reinterpret_cast<uintptr_t**>(entity);

	std::cout << "vtable pointer - 0x" << std::uppercase << std::hex << vtable_ptr << std::endl;
	std::cout << "print function = 0x" << std::uppercase << std::hex << *vtable_ptr << std::endl;
	std::cout << "get_name = 0x" << std::uppercase << std::hex << *(vtable_ptr + 1) << std::endl;
}

uintptr_t** class_pointer = nullptr;
uintptr_t* original_vtable_pointer = nullptr;
const auto index = 0;

void __fastcall hk_print(entity_t* ecx, void* edx, std::string name)
{
	static auto fn = reinterpret_cast<void(__thiscall*)(entity_t*, std::string)>(original_vtable_pointer[index]);
	fn(ecx, "Hooked");
}

uintptr_t* setup_vtable_hook(entity_t* entity)
{
	// Single hook
	//uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(entity);

	//uint32_t old;
	//VirtualProtect(&vtable[0], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, reinterpret_cast<PDWORD>(&old));

	//vtable[0] = reinterpret_cast<uintptr_t>(hk_print);

	//VirtualProtect(&vtable[0], sizeof(uintptr_t), old, reinterpret_cast<PDWORD>(&old));

	class_pointer = reinterpret_cast<uintptr_t**>(entity);
	original_vtable_pointer = *class_pointer;

	size_t num_virtual_methods = 0;
	while (reinterpret_cast<uintptr_t*>(original_vtable_pointer[num_virtual_methods])) 
	{
		num_virtual_methods++;
	}

	auto new_vtable_pointer = new uintptr_t[num_virtual_methods];
	memcpy(new_vtable_pointer, original_vtable_pointer, num_virtual_methods * sizeof(uintptr_t));
	new_vtable_pointer[index] = reinterpret_cast<uintptr_t>(&hk_print);

	return new_vtable_pointer;
}

void enable_vtable_hook(uintptr_t* vtable_pointer)
{
	*class_pointer = vtable_pointer;
}

void unhook_vtable()
{
	*class_pointer = original_vtable_pointer;
}

int main()
{
	entity_t* entity = new entity_t();
	entity_t* player = new player_t();

	std::cout << "===============================" << std::endl;

	std::cout << "entity address = 0x" << std::uppercase << std::hex << &entity << std::endl;
	std::cout << "player address = 0x" << std::uppercase << std::hex << &player << std::endl;

	std::cout << "===============================" << std::endl;

	std::cout << "Printing vtable:" << std::endl << std::endl;
	print_vtable(entity);

	std::cout << "===============================" << std::endl;

	auto new_vtable_pointer = setup_vtable_hook(entity);
	std::cout << "new_vtable_pointer = 0x" << std::uppercase << std::hex << &entity << std::endl;

	std::cout << "===============================" << std::endl;

	std::cout << "Hooking vtable:" << std::endl << std::endl;

	enable_vtable_hook(new_vtable_pointer);

	entity->print("Idiot 1");
	player->print("Idiot 2");

	std::cout << "===============================" << std::endl;

	std::cout << "Unhooking vtable:" << std::endl << std::endl;
	unhook_vtable();

	entity->print("Idiot 1");
	player->print("Idiot 2");

	std::cout << "===============================" << std::endl;

	std::cout << "Hooking vtable:" << std::endl << std::endl;
	enable_vtable_hook(new_vtable_pointer);

	entity->print("Idiot 1");
	player->print("Idiot 2");

	std::cout << "===============================" << std::endl;

	delete entity;
	delete player;
	delete new_vtable_pointer;

	std::cin.get();
}