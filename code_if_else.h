#ifndef __CODE_IF_ELSE_H__
#define __CODE_IF_ELSE_H__

#include <stdio.h>
#include <vector>

#include "code_element.h"

class code_if_else : public code_element
{
	public:
		code_if_else();
		~code_if_else();
		void fprint(FILE *dest, int depth);
		void add_lcb(code_element *add);
		void add_ecb(code_element *add);
		void set_else(code_element *h);
		void set_last(code_element *l);
	private:
		std::vector<code_element*>lcb;	//the logic blocks
		std::vector<code_element*>ecb;	//the executing blocks
		code_element *helse;	//the else block
		code_element *last;	//the block everything merges to
};

#endif