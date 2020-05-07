#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "structs.h"


Process* process_init(uint8_t n_burst)
{
  // Creo la lista ligada
  Process* p = malloc(sizeof(Process));
  // Pongo sus punteros en nulo y su contador en 0
  p -> pid = 0;
  p -> name = (char*)malloc(32*sizeof(char));
  p -> actual;
  p -> n_burst = n_burst;
  p -> bursts = (uint8_t*)calloc(2*n_burst-1, sizeof(uint8_t));
  p -> turnaround_time = 0;
  p -> response_time = 0;
  p -> waiting_time = 0;
  p -> time_left_burst = 0;
  p -> time_left = 0;
  p -> actual_burst = 0;
  p -> chosen = 0;
  p -> interrupted = 0;

  // Retorno la lista ligada
  return p;
}

void process_destroy(Process* p)
{
  free(p->name);
  //for (size_t i = 0; i <2*p->n_burst - 1; i++) {
  //  free(&p->bursts[i]);
  //}
  free(p->bursts);
  free(p);
}

Queu* q_init(uint8_t n_processes){
  Queu* q = malloc(sizeof(Queu));
  q -> p_pointer = ll_init();
  q -> all_p = (Process*)calloc(n_processes,sizeof(Process));
  return q;
}

void queu_destroy(Queu* q, uint8_t n_processes) {
  ll_destroy(q -> p_pointer);
  //for (size_t i = 0; i < n_processes; i++) {
  //  process_destroy(q->all_p[i]);
  //}
  free(q->all_p);
  free(q);
}

////// Funciones privadas ///////

/** Crea un nodo a partir de un valor */
static Node* node_init(Process* value)
{
  // Pido memoria para el nodo
  Node* node = malloc(sizeof(Node));

  // Le asigno el valor correspondiente
  node -> value = value;

  // Hago que apunte al null
  node -> next = NULL;

  // Retorno el nodo
  return node;
}

/** Libera iterativamente la memoria del nodo y los nodos siguientes */
static void node_destroy(Node* node)
{
  // Esta funcion es iterativa porque puede generar stack overflow
  while (node)
  {
    Node* next = node -> next;
    free(node);
    node = next;
  }
}

// Agrega un nodo a la lista ligada
static void ll_add_node(LinkedList* ll, Node* node)
{
  if (!ll -> head)
  {
    // Si la lista no tiene elementos, el nodo es inicial y final
    ll -> head = node;
    ll -> tail = node;
  }
  else
  {
    // Sino, lo agrego al final
    ll -> tail -> next = node;
    ll -> tail = node;
  }
  ll -> size++;
}

////// Funciones publicas ///////

/** Crea una lista ligada vacia */
LinkedList* ll_init()
{
  // Creo la lista ligada
  LinkedList* ll = malloc(sizeof(LinkedList));

  // Pongo sus punteros en nulo y su contador en 0
  ll -> head = NULL;
  ll -> tail = NULL;
  ll -> size = 0;

  // Retorno la lista ligada
  return ll;
}

/** Destruye la lista ligada */
void ll_destroy(LinkedList* ll)
{
  // Primero destruyo los nodos de la lista
  node_destroy(ll -> head);

  // Luego libero la memoria de la lista
  free(ll);
}

/** Agrega un valor a la lista */
void ll_append(LinkedList* ll, Process* value)
{
  // Creo el nodo a insertar
  Node* node = node_init(value);

  // Funcion que agrega un nodo al final de una lista ligada
  ll_add_node(ll, node);
}

void ll_clear(LinkedList* ll){
  node_destroy(ll->head);
  //ll -> head = NULL;
  //ll -> tail = NULL;
  Node* head;
  Node* tail;
  ll-> head = head;
  ll-> tail = tail;
  ll -> size = 0;
}

/** Saca el primer valor de la lista */
void ll_pop(LinkedList* ll){
  if (ll->head !=NULL){
    Node* first = ll -> head;
    if (ll->head->next != NULL) {
      ll -> head = first -> next;
    }
    else{
      ll -> head = NULL;
    }
    free(first);
    ll->size--;
  }
}

void ll_insert(Process* p, Node* prev, LinkedList* ll, int location){

/*si le paso la cabeza y es menor a la cabeza*/
  if (prev == ll->head && location == 0) {
    if (p->time_left<prev->value->time_left) {

      Node* new = node_init(p);
      new->next = ll->head;
      ll->head = new;
      ll->size++;

    }
    else{
      ll_insert(p,prev,ll,1);
    }
  }

/*si le paso la cabeza y no es menor a la cabeza*/
  else{
    if(prev->next!=NULL){
      if (p->time_left<prev->next->value->time_left) {
        Node* new = node_init(p);
        new->next = prev->next;
        prev->next = new;
        ll->size++;

      }
      else if (p->time_left==prev->next->value->time_left) {
        Node* new = node_init(p);
        if (p->time_left_burst<prev->next->value->time_left_burst) {
          new->next = prev->next;
          prev->next = new;
          ll->size++;
        }
        else{
          new->next = prev->next->next;
          prev->next->next = new;
          ll->size++;
        }
      }
      else{
        ll_insert(p,prev->next,ll,1);
      }
    }


    else{
      Node* new = node_init(p);
      prev->next = new;
      ll->tail = new;
      ll->size++;

    }

  }
}
