//
// Binary Space Partitioning demo
// 1st v. Dr Anton Gerdelan 20 Oct 2016 <gerdela@scss.tcd.ie>
// Written in C99
//
// Summary of algorithm:
// 1. pick any line/wall/polygon to be 'root' node
// 2. create 2 child lists off root; infront and behind
// 3. compare all other lines to root based on their position vs.
//    root's normal. Put them into the lists.
// 4. process infront then behind list: choose a root node and recurse
//    until lists are empty and only nodes remain

#include <assert.h> // assert()
#include <stdbool.h> // bool data type
#include <stdio.h> // printf()
#include <stdlib.h> // malloc()
#include <string.h> // memset()

typedef struct BSP_Tree_Node BSP_Tree_Node;

//
// lists of walls that are not yet sorted into tree nodes
// this should be dynamic memory but i will do the lazy way first
typedef struct BSP_List {
	int items[128];
	int count;
} BSP_List;

//
// this is a struct for a node in my BSP tree
struct BSP_Tree_Node {
	int line_index;
	// TODO do lists need to be here or can be in function or global?
	// i think these can be function arguments because they will be
	// used in a recursive sorting/storing function
	BSP_List list_infront, list_behind;
	// this is a binary tree with up to 2 children
	BSP_Tree_Node *node_infront, *node_behind;
};

//
// add a line (wall) to a list of unsorted lines
bool add_to_list( BSP_List *list, int index ) {
	assert( list );
	if ( list->count >= 128 || list->count < 0 ) {
		fprintf( stderr, "ERROR: list full\n" );
		return false;
	}
	list->items[list->count] = index;
	list->count++;
	return true;
}

//
// create a tree from a list of lines
// i'm thinking this can probably be recursive to
// create sub-trees too, but we'll see
BSP_Tree_Node *create_bsp( BSP_List *list ) {
	assert( list );
	BSP_Tree_Node *root = NULL;
	if ( list->count > 0 ) {
		root = (BSP_Tree_Node *)malloc( sizeof( BSP_Tree_Node ) );
	}
	return root;
}

int main() {
	BSP_List original_list;
	memset( original_list.items, -1, 128 );

	BSP_Tree_Node *root = create_bsp( &original_list );

	return 0;
}
