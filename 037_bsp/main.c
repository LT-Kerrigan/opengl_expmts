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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct BSP_Tree_Node BSP_Tree_Node;

typdef struct BSP_List{
	int items[128];
	int count;
}BSP_List;

// TODO do lists need to be here or can be in function or global?
struct BSP_Tree_Node{
	int line_index;
	BSP_List list_infront, list_behind;
	BSP_Tree_Node *node_infront, *node_behind;
};

bool add_to_list(BSP_List* list, int index){
	assert(list);
	if(list->count >= 128 || list->count < 0){
		fprinf(stderr, "ERROR: list full\n");
		return false;
	}
	list->items[list->count] = index;
	list->count++;
	return true;
}

BSP_Tree_Node* create_bsp(BSP_List* list){
	assert(list);
	BSP_Tree_Node* root = NULL;
	if(list->count > 0){
		root = (BSP_Tree_Node*)malloc(sizeof(BSP_Tree_Node));
		
	}
	return root;
}

int main(){
	BSP_List original_list;
	memset(original_list->items, -1, 128);

	BSP_Tree_Node* root = create_bsp(&original_list);

	return 0;
}
