#include <cstring>
#include <sys/stat.h>

#include "disassembly/disassembler.h"
#include "exceptions.h"
#include "executable.h"
#include "exe_elf.h"
#include "exe_macho.h"
#include "exe_pe.h"

executable::executable()
{
	rbo = 0;
	exe_file = new std::ifstream;
	exe_type = EXEC_TYPE_UNKNOWN;
	exe_object = 0;
}

executable::~executable()
{
	delete exe_object;
	delete exe_file;
}

int executable::check_pe(std::istream *me)
{
	unsigned int signature;
	signature = 0;
	me->seekg(0x3C, std::ios::beg);
	me->read((char*)&signature, 4);
	if (me->fail())
	{
		std::cout << "error reading offset location\n";
		return -1;
	}
	me->seekg(signature, std::ios::beg);
	if (me->good())
	{
		me->read((char*)&signature, 4);
		if (signature == 0x4550)
		{
			std::cout << "PE executable detected\n";
			exe_type = EXEC_TYPE_PE;
			return 1;
		}
		else if (signature == 0x00004550)
		{
			std::cout << "BACKWARDS? PE executable detected\n";
			exe_type = EXEC_TYPE_PE;
			rbo = 1;
			return 1;
		}
	}

	return 0;
}

int executable::output(const char *fld_name)
{
	folder = fld_name;
	int status = mkdir(fld_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == -1)
	{
		switch (errno)
		{
		case EEXIST:
			break;
		default:
			std::cout << "Status: " << status << " " << errno << std::endl;
			throw "Could not create output folder";
			break;
		}	
	}
	return status;
}

void executable::set_name(const char* n)
{
	std::string temp(n);
	int folder_index = 0;
	for (unsigned int i = 0; i < strlen(n); i++)
	{
		if (n[i] == '/')
		{
			folder_index = i+1;
		}
	}
	exe_name = temp.substr(folder_index, temp.size() - folder_index);
}

std::string executable::get_name()
{
	return exe_name;
}

std::vector<source_file*> executable::get_sources()
{
	return sources;
}

void executable::write_sources(std::string n)
{
	std::string temp(n + "/" + exe_name);
	
	int status = mkdir(temp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == -1)
	{
		switch (errno)
		{
		case EEXIST:
			break;
		default:
			std::cout << "Status: " << status << " " << errno << std::endl;
			throw "Could not create output folder";
			break;
		}	
	}
	
	for (unsigned int i = 0; i < sources.size(); i++)
	{
		sources[i]->write_sources(n);
	}
}

int executable::load(const char *bin_name)
{
	int processed = 0;
	exe_file->open(bin_name, std::ios::binary);
	if (!exe_file->is_open())
	{
		std::cout << "Failed to open executable " << bin_name << "\n";
		throw file_open_failed(bin_name);
	}

	int reverse = exe_elf::check(exe_file);
	if (reverse != 0)
	{
		exe_type = EXEC_TYPE_ELF;
		exe_object = new exe_elf();
		if (reverse < 0)
			rbo = 1;
	}
	reverse = exe_macho::check(exe_file);
	if (reverse != 0)
	{
		exe_type = EXEC_TYPE_MACHO32;
		exe_object = new exe_macho();
		if (reverse < 0)
			rbo = 1;
	}
	reverse = exe_pe::check(exe_file);
	if (reverse != 0)
	{
		exe_type = EXEC_TYPE_PE;
		exe_object = new exe_pe();
		if (reverse < 0)
			rbo = 1;
	}

	if (exe_type == EXEC_TYPE_UNKNOWN)
	{
		exe_file->close();
		throw unknown_exe_format(bin_name);
	}

	if (exe_object == 0)
	{
		exe_file->close();
		throw unknown_exe_format(bin_name);
	}
	if (exe_object->process(exe_file) != 0)
	{
		exe_file->close();
		throw unknown_exe_format(bin_name);
	}

	function *start;
	std::vector<address> function_addresses;	//a list of function start addresses
	start = new function(exe_object->entry_addr(), "void", exe_object->entry_name(), *(exe_object->get_disasm()));
	source_file *nsf = new source_file(exe_name + "/" + start->get_name() + ".c");
	nsf->add_function(start);
	sources.push_back(nsf);
	start->output_graph_data(folder);
	std::vector<address> temp = sources.back()->get_calls();
	std::cout << std::hex;
	for (unsigned int i = 0; i < temp.size(); i++)
	{
		if (!check_func_list(temp[i]))
		{
			std::cout << "\tNew function at 0x" << temp[i] << std::endl;
			function_addresses.push_back(temp[i]);
		}
	}
	while (function_addresses.size() > 0)
	{
		std::stringstream name;
		name << "func_" << std::hex << function_addresses[0] << std::dec;
		function *tfunc = new function(function_addresses[0], "unknown", name.str().c_str(), *(exe_object->get_disasm()));
		source_file *tfsf = new source_file(exe_name + "/" + tfunc->get_name() + ".c");
		tfsf->add_function(tfunc);
		sources.push_back(tfsf);
		tfunc->output_graph_data(folder);
//		tfunc->output_code(folder);
		function_addresses.erase(function_addresses.begin());
		std::vector<address> temp = sources.back()->get_calls();
		std::cout << std::hex;
		for (unsigned int i = 0; i < temp.size(); i++)
		{
			if (!check_func_list(temp[i]))
			{
				std::cout << "\tNew function at 0x" << temp[i] << std::endl;
				function_addresses.push_back(temp[i]);
			}
		}
	}

	exe_file->close();
	return processed;
}

bool executable::check_func_list(address addr)
{
	for (unsigned int i = 0; i < sources.size(); i++)
	{
		if (sources[i]->find_function(addr))
			return true;
	}
	return false;
}
