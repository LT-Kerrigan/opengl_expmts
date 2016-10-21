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
//
// Assumptions:
// * I'm not actually going to load a map file (yet) - just hard-code some walls
// * A proper BSP algorithm will split larger walls that are both in-front and
// behind
//   some root wall. I won't bother with this yet - I'll just default to in-front in
//   this case.
//

#include <assert.h>	// assert()
#include <stdbool.h> // bool data type
#include <stdio.h>	 // printf()
#include <stdlib.h>	// malloc()
#include <string.h>	// memset()

// counter to check how many nodes are in the tree
int g_nodes_in_tree;

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
	// this is a binary tree with up to 2 children
	BSP_Tree_Node *child_infront, *child_behind;
};

//
// a wall
typedef struct Line {
	// start and end points of wall in 2d
	float start_x, start_y;
	float end_x, end_y;

	// the facing direction as a 2d unit vector
	// could also have been an angle in e.g. degrees
	float normal_x, normal_y;
} Line;

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
// recursively create a tree from a list of lines
// this could also be a loop of course - better for bigger maps
BSP_Tree_Node *create_bsp( BSP_List list ) {
	// if nothing left to sort - break out of recursion
	if ( list.count == 0 ) {
		return NULL;
	}
	BSP_Tree_Node *root = NULL;
	root = (BSP_Tree_Node *)malloc( sizeof( BSP_Tree_Node ) );
	root->line_index = -1;
	root->child_infront = NULL;
	root->child_behind = NULL;
	g_nodes_in_tree++;

	//
	// TODO -- create 2 new lists for infront and behind
	//

	//
	// TODO -- recurse here with new lists
	//

	return root;
}

int main() {
	// My map is just a list of walls
	// I'll hard-code the walls manually first
	BSP_List original_list;
	{ // reset the list and then add some walls
		original_list.count = 0;
		memset( original_list.items, -1, 128 ); // i'll use index -1 to mean nothing
	}
	BSP_Tree_Node *root = create_bsp( original_list );

	printf( "tree created with %i nodes\n", g_nodes_in_tree );

	return 0;
}
