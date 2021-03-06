#include "function.h"

#include <iostream>
#include <fstream>
#include <string>

#include "exceptions.h"

function::function(address addr, const char *t, const char *n,
	std::vector<code_element *> a)
	: name(n), ret_type(t), code(a)
{
	s = addr;
}

function::~function()
{
}

address function::gets()
{
	return s;
}

void function::set_name(const char *to)
{
	name = to;
}

void function::set_type(type t)
{
	ret_type = t;
}

std::string function::get_name()
{
	return name;
}

void function::simplify()
{
	code.simplify();
}

std::ostream& operator << (std::ostream& output, function &me)
{
	me.fprint(output);
	return output;
}

void function::get_calls(std::vector<address> &c)
{
	code.get_calls(c);
}

void function::fprint(std::ostream &output)
{	//print the code to the output for examination
	unsigned int i;
	//output << "//There are " << pieces.size() << " blocks\n";
	output << ret_type.get_name() << " " << name << "(";

	if (arguments.size() > 0)
	{
		output << arguments[0];
	}
	for (i = 1; i < arguments.size(); i++)
	{
		output << ", " << arguments[i];
	}

	output << ")\n{\n";
	code.fprint(output, 1);
	output << "}\n";
}

void function::output_graph_data(std::string fld_name)
{
	std::string outname = fld_name + "/" + name + ".gv";
	std::ofstream output;
	output.open(outname, std::ios::out);
	output << "digraph " << name << "{\n";
	output << std::hex;
	code.print_graph(output);
	output << "}\n";
	output.close();
	
	simplify();
	
	outname = fld_name + "/" + name + "sim.gv";
	output.open(outname, std::ios::out);
	output << "digraph " << name << "{\n";
	output << std::hex;
	code.print_graph(output);
	output << "}\n";
	output.close();
}

