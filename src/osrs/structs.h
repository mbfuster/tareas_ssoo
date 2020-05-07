#pragma once
enum state {RUNNING, READY, WAITING, FINISHED};

typedef struct process {
   uint32_t  pid;
   char* name;
   enum state actual;
   uint8_t time_of_arrival;
   uint8_t n_burst;
   uint8_t* bursts;
   uint8_t turnaround_time;
   uint8_t response_time;
   uint8_t waiting_time;
   int time_left_burst;
   int time_left;
   int actual_burst;
   int chosen;
   int interrupted;
} Process;

/** Estructura de un nodo de la lista simplemente ligada */
typedef struct node
{
  /** Nodo sigiente en la lista (NULL si no hay otro nodo) */
  struct node* next;
  /** valor que guarda el nodo */
  Process* value;
} Node;

/** Lista ligada. Mantiene el nodo inicial y el final */
typedef struct linked_list
{
  /** Nodo inicial de la lista (posicion 0) */
  Node* head;
  /** Nodo final de la lista */
  Node* tail;
  /*size de la lista*/
  uint8_t size;
} LinkedList;

typedef struct queue{
  /*Solo procesos READY*/
  LinkedList* p_pointer;
  Process* all_p;
} Queu;



/////// Funciones publicas de la lista ////////

/** Crea una lista ligada vacia */
LinkedList* ll_init();

/** Destruye la lista ligada */
void ll_destroy(LinkedList* ll);

/** Agrega un valor a la lista */
void ll_append(LinkedList* ll, Process* value);

/**Limpiar la lista sin destruirla*/
void ll_clear(LinkedList* ll);

Process* process_init(uint8_t n_burst);

void process_destroy(Process* p);

Queu* q_init(uint8_t n_processes);

void ll_pop(LinkedList* ll);

void ll_insert(Process* p, Node* prev, LinkedList* ll, int location);

void queu_destroy(Queu* q, uint8_t n_processes);
