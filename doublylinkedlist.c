/* Yanling Wang Fall 2018 */

#include <stdio.h>
#include <stdlib.h>
#include "doublylinkedlist.h"

void insertHead(doublyLinkedList *listPtr, int value) {

  
 
// listNode *head = listPtr->head;
// listNode *tail = listPtr->tail;
  listNode* Nnode = ( listNode*)malloc(sizeof( listNode));
   
 
  Nnode->value = value;
  
  if (listPtr->head == NULL){
    
    listPtr->head = Nnode;
    
    listPtr->tail =Nnode; 
    return;  
  }
   
// change value of the head and tail to new node if head pointer is 0

  Nnode->next =listPtr-> head;
  Nnode->prev = NULL;
  listPtr->head -> prev = Nnode;
  listPtr->head = Nnode;
//change previous of head to new node and head to New node
}

int removeTail(doublyLinkedList *listPtr) {
// listNode *tail = listPtr ->tail;
// listNode *head = listPtr ->head;
 // listNode *temp;
 // int y;
  //  y =  tail -> value;
    if (listPtr -> tail == NULL){

    return -1;
    }
    int newv= listPtr->tail->value;   
   listNode *current = listPtr -> tail; 
    if (listPtr ->tail -> prev == NULL) {

        listPtr -> tail = NULL;
        listPtr -> head = NULL;// setting both to null for the single node left
        }

       else{
    listPtr -> tail = listPtr -> tail -> prev;
    listPtr -> tail -> next = NULL;}
   free(current);
  return newv; 
}
void showHead(doublyLinkedList *listPtr) {
  listNode *head = listPtr->head;
  printf("list head: ");
  while (head != NULL) {
    printf("%d ", head->value);
    head = head->next;
  } 
  printf("nil\n");
  return;
} 
void showTail(doublyLinkedList *listPtr) {
  listNode *tail = listPtr->tail;
  printf("list tail: ");
  while (tail != NULL) {
    printf("%d ", tail->value);
    tail = tail->prev;
  } 
  printf("nil\n");
  return;
} 
void freeList(doublyLinkedList *listPtr) { 
  listNode *head = listPtr->head;
  listNode *temp;
  while(head != NULL) {
    temp = head->next;
    free(head);
    head = temp;
  }
  listPtr->head = NULL;
  listPtr->tail = NULL;
  return;
}

