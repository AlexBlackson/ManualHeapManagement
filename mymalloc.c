/*
 * Name: Alex Blackson
 * CS 449: Systems Software
 * Professor Wonsun Ahn
 * Last Edited: 11/13/17
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

struct Block {
  int occ;              // whether block is occupied
  int size;             // size of block (including header)
  struct Block *prev;   // pointer to previous block
  struct Block *next;   // pointer to next block
};

static struct Block *head = NULL;       // head of list
static int nodeCount = 0; 						// total number of nodes in list

void *my_malloc(int size)
{
  struct Block* currNode = NULL;
  
  // If there are no nodes allocated on heap
  if (head == NULL){
	  head = sbrk(sizeof(struct Block) + size);
	  head->occ = 1;
	  head->size = size + sizeof(*head);
	  head->prev = NULL;
	  head->next = NULL;
	  currNode = head;
	  nodeCount = nodeCount + 1;
  }
  
  // If there is memory allocated on heap
  else{
	  currNode = head;
	  struct Block *prevNode = head;
	  // Allocates enough block pointers as there are blocks
	  struct Block *possibleFits [nodeCount];
	  // 0 when at the first node
	  int first = 0;
	  int fitCount = 0;
	  
	  while(currNode != NULL){
		if (currNode->occ == 0){
			// Adds nodes to list of possible fits when appropriate
			if (currNode->size >= (size + sizeof(struct Block))){
				possibleFits[fitCount] = currNode;
				fitCount++;
			}
		}
		if (first == 0){
			currNode = currNode->next;
			first = 1;
		}
		else{
			currNode = currNode->next;
			prevNode = prevNode->next;
		}
	  }
	  
	  // If there are no available fits, the heap is expanded
	  if (fitCount == 0){
		  currNode = sbrk(sizeof(struct Block) + size);
		  prevNode->next = currNode;
		  currNode->size = size + sizeof(*currNode);
		  currNode->occ = 1;
		  currNode->prev = prevNode;
		  currNode->next = NULL;
		  nodeCount = nodeCount + 1;
	  }
	  
	  // Uses best-fit algorithm to find the best fit
	  else{
		struct Block *bestFit = possibleFits[0];
		int minSize = bestFit->size;
		int i;
		
		for (i = 1; i < fitCount; i++){
			if (possibleFits[i]->size < minSize){
				bestFit = possibleFits[i];
				minSize = bestFit->size;
			}
		}
		currNode = bestFit;
		
		// Case when the best-fit is exactly the size of size to allocate
		if (currNode->size == (size + sizeof(struct Block))){
			currNode->occ = 1;
		}
		
		
		// Splits large block into smaller blocks
		if(currNode->size > (size+sizeof(struct Block))){
			void *temp = ((void *)currNode) + size + sizeof(struct Block);
			struct Block *insertNode = temp;
			
			insertNode->size = currNode->size - (size + sizeof(struct Block));	
			currNode->size = size+sizeof(struct Block);
			
			insertNode->next = currNode->next;
			insertNode->prev = currNode;
			currNode->next = insertNode;
			
			insertNode->occ = 0;
			currNode->occ = 1;
			
			if (insertNode->next != NULL){
				insertNode->next->prev = insertNode;
			}
			nodeCount = nodeCount + 1;
		}
	  }
  }
  return currNode + 1;
}

void my_free(void *data)
{
	struct Block *search = data - sizeof(struct Block);
	search->occ = 0;
	
	
	// If previous node is empty 
	if(search->prev != NULL){
		if (search->prev->occ == 0){
			search->prev->size = search->size + search->prev->size;
			search->prev->next = search->next;
			if(search->next != NULL){
				search->next->prev = search->prev;
			}
			search = search->prev;
			nodeCount = nodeCount - 1;
		}
	}
	
	//If next node is empty
	if(search->next != NULL){
		if (search->next->occ == 0){
			search->size += search->next->size;
			search->next = search->next->next;
			
			if (search->next != NULL){
				search->next->prev = search;
			}
			nodeCount = nodeCount - 1;
		}
	}
	
	//If node to remove is the last node
	if (search->next == NULL){
		// Deallocating first element
		if (search->prev == NULL){
			sbrk(-(search->size));
			head = NULL;
		}
		// Deallocating last element
		else{
			search->prev->next = NULL;
			sbrk(-(search->size));
		}
		nodeCount = nodeCount - 1;
	}
}

// Copied from given heap.c file
void dump_heap()
{
  struct Block *cur;
  printf("brk: %p\n", sbrk(0));
  printf("head->");
  for(cur = head; cur != NULL; cur = cur->next) {
    printf("[%d:%d:%d]->", cur->occ, (char*)cur - (char*)head, cur->size);
    fflush(stdout);
    assert((char*)cur >= (char*)head && (char*)cur + cur->size <= (char*)sbrk(0)); // check that block is within bounds of the heap
    if(cur->next != NULL) {
      assert(cur->next->prev == cur); // if not last block, check that forward/backward links are consistent
      assert((char*)cur + cur->size == (char*)cur->next); // check that the block size is correctly set
    }
  }
  printf("NULL\n");
}
