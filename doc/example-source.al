import (mul ref_get_refered call_macro to_node node main external cond_branch cmp eq add let return call unique int macro proc list_get list_push list_create lit_create) from "core";
export int64 some_macro doubled_int64 test asdfasdf plus2_proc;
export plus_2_proc plus_1_proc;

# this macro takes its first argument and simply returns it
def alias1 macro # alias1 is the name of the defined symbol, macro a reference to macro imported from "core"
((s node)) # list of function arguments: only a single argument s of type node
node
{	# begin of block list
	block1 # first block name
	{
		let elem1 list_get s 0; # elem1 is the first node in the node list s
		(return (node)) elem1; # return elem1
	};
};

def int64 alias1 (int 64); # int64 should be defined as (int 64)
def int32 alias1 (int 32); 
def return_node alias1 (return node);

# this macro returns a list containing the macro's first argument twice
def some_macro macro ((s node)) node
{
	block1
	{
		let result (list_create); # create empty list
		let first (list_get) s 0; # get first macro argument
		(list_push) result first; # push first argument at end of list
		(list_push) result first; 
		(return node) result;
	};
};

# slightly modified some_macro
def some_macro2 macro ((s node)) node
{
	block1
	{
		let result (list_create);
		let second (list_get) s 1;
		(list_push) result second;
		(list_push) result second;
		(list_push) result second;
		(return node) result;
	};
};

def asdfasdf some_macro2 () "fff";

def doubled_int64 some_macro int64;

# a proc that takes two arguments of type int64 and returns their sum (int64 was defined above)
def add_proc proc ((a int64)(b int64)) int64
{
	block1
	{
		let result (add int64) a b;
		(return int64) result;
	};
};

# macro to show branching: should always return from block2
def macro2 macro ((s node)) node
{
	block1
	{
		let x (call (int64 int64) int64) add_proc 5 10; # call add_proc defined above
		let c (cmp eq int64) x 15; # compare x to 15 (true if eq-ual)
		(cond_branch) c block2 block3; # jump to block2 if c is true, go to block3 otherwise
	};
	block2
	{
		let y (list_create);
		(return node) y; # return empty list
	};
	block3
	{
		let z (lit_create);
		(return node) z; # return empty lit (this should never happen)
	};
};

def test macro2; # should be an emtpy list

def int8 alias1 (int 8);
def print_char external "print_char" (int8) int8; # declare an external function that is linked into the executable, but not visible to the compiler
def call_int8_int8 alias1 (call (int8) int8);


def int8_int8_call alias1 (call (int8) int8);
def return_int8 alias1 (return int8);

# this macro takes two procs with signature "int8 -> int8" and composes them, returning a new proc
def compose_int8_procs macro ((args node)) node
{
	entry
	{
		# to_node's of some nodes used below (and defined above)
		let let to_node let;
		let call_int8_int8 to_node call_int8_int8;
		let a to_node a;
		let b to_node b;
		let c to_node c;
		let entry to_node entry;
		let return_int8 to_node return_int8;

		# extract entry procs from args
		let first_arg list_get args 0;
		let second_arg list_get args 1;

		# create params and return type nodes
		let params to_node ((a int8));
		let return_type to_node int8;
		
		# create statements in block
		let first_statement list_create;
		list_push first_statement let;
		list_push first_statement b;
		list_push first_statement call_int8_int8;
		list_push first_statement second_arg; # we have to apply the right-hand side proc first
		list_push first_statement a;

		let second_statement list_create;
		list_push second_statement let;
		list_push second_statement c;
		list_push second_statement call_int8_int8;
		list_push second_statement first_arg;
		list_push second_statement b;

		let third_statement list_create;
		list_push third_statement return_int8;
		list_push third_statement c;

		# create block body
		let block_body list_create;
		list_push block_body first_statement;
		list_push block_body second_statement;
		list_push block_body third_statement;

		# create entry block
		let entry_block list_create;
		list_push entry_block entry;
		list_push entry_block block_body;

		# create function body
		let function_body list_create;
		list_push function_body entry_block;

		# create proc source
		let proc_source list_create;
		list_push proc_source params;
		list_push proc_source return_type;
		list_push proc_source function_body;

		# compile proc
		let compiled_proc call_macro proc proc_source;
		
		return_node compiled_proc;
	};
};


def plus_6_proc proc ((a int8)) int8
{
	entry
	{
		let result (add int8) a 6;
		(return int8) result;
	};
};
def square_proc proc ((a int8)) int8
{
	entry
	{
		let result (mul int8) a a;
		(return int8) result;
	};
};

def squared_plus_6 compose_int8_procs plus_6_proc square_proc;

def m main () int32
{
	entry
	{
		let should_be_42 (call (int8) int8) squared_plus_6 6; # check if plus_2 works correctly
		let check (cmp eq int8) should_be_42 42;
		cond_branch check hello_world nothing;
	};
	hello_world
	{
		# prints "Hello World!\n"
		call_int8_int8 print_char 72;
		call_int8_int8 print_char 101;
		call_int8_int8 print_char 108;
		call_int8_int8 print_char 108;
		call_int8_int8 print_char 111;
		call_int8_int8 print_char 32;
		call_int8_int8 print_char 87;
		call_int8_int8 print_char 111;
		call_int8_int8 print_char 114;
		call_int8_int8 print_char 108;
		call_int8_int8 print_char 100;
		call_int8_int8 print_char 33;
		call_int8_int8 print_char 10;
		(return (int 32)) 0;
	};
	nothing
	{
		(return (int 32)) 1;
	};
};
