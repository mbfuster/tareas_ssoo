#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>
#include "structs.h"
#include "usefull_func.h"

int main(int argument_count, char* arguments[])
{
  if (argument_count < 3)
	{
		printf("Modo de uso: %s <file> <output> <version> [<quantum>]\n", arguments[0]);
		return 1;
	}
  else
  {
/*---------------------------------------------------------------------------*/
/*---------------------- Setear variables y crear structs--------------------*/
/*---------------------------------------------------------------------------*/
    char* inputname = arguments[1];
    char* outputname = arguments[2];
    char* version = arguments[3];
    int quantum;
    if (arguments[4]) {
      quantum = atoi(arguments[4]);
    }
    else{
      quantum = 5;
    }

    //leer archivo
    FILE* file = fopen(inputname, "r");
    if( file == NULL ) {
       perror("Custom error alert: Error ");
       return -1;
    }

    uint8_t n_processes;
    fscanf(file,"%hhd",&n_processes);

    //lista con punteros a todos los struct process
    Process* all_p[n_processes];

    //definir variables locales para llenar structs
    printf("%hhd\n",n_processes);
    char name_p[32];
    uint8_t time_of_arrival;
    uint8_t n_burst;



    //Definir valores para cada struct
    for (int i = 0; i < n_processes; i++) {
      fscanf(file, "%s %hhd %hhd", name_p,&time_of_arrival,&n_burst);

      uint8_t bursts[2*n_burst - 1];
      for (size_t i = 0; i < 2*n_burst-1; i++) {
        fscanf(file,"%hhd", &bursts[i]);
      }

      Process* p = process_init(n_burst);
      all_p[i] = p;
      all_p[i] -> time_of_arrival = time_of_arrival;
      strcpy(all_p[i]->name, name_p);
      for (size_t j = 0; j < 2*n_burst-1; j++) {
        all_p[i] -> bursts[j] = bursts[j];
        if (j%2==0) {
          all_p[i] -> time_left += bursts[j];
        }
      }
      all_p[i] -> pid = i+1;

    }
    fclose(file);

    for (size_t i = 0; i < n_processes; i++) {
      printf("%s,%d,%d\n",all_p[i]->name, all_p[i]->time_of_arrival,all_p[i]->bursts[0]);
    }


/*---------------------------------------------------------------------------*/
/*-------------------------- S I M U L A C I O N ----------------------------*/
/*---------------------------------------------------------------------------*/

    Queu* q = q_init(n_processes);
    for (size_t j = 0; j < n_processes; j++) {
      q->all_p[j] = *(all_p[j]);
    }
    //borrar despues

    uint8_t default_val = 1;
    Process* idle = process_init(default_val);
    strcpy(idle->name, "idle");
    idle -> pid = 0;
    idle -> actual = RUNNING;
    idle -> bursts[0]=255;
    idle -> time_left;
    int n_finished = 0;
    int timer = 0;
    Process* on_cpu = idle;
    int arrival_ptr = 0;
    int left_quantum = quantum;

    while (n_finished < n_processes) {

/*----------------------------Version preemptive------------------------------*/
      if (!strcmp(version, "p")) {
        printf("***********************************************************\n");
        printf("Inicio de periodo\n");
        printf("-----------------\n");


        printf("timer:%d, %s,le queda %d\n",timer,on_cpu->name, on_cpu->time_left_burst);
        for (size_t i = 0; i < n_processes; i++){
          if (q->all_p[i].actual==FINISHED) {
            printf("-----%s ya termino con turnaround_time:%d\n",q->all_p[i].name,q->all_p[i].turnaround_time);
          }
          printf("nombre:%s,estado:%d, time_left:%d\n",q->all_p[i].name,q->all_p[i].actual,q->all_p[i].time_left);
        }
        printf("terminados: %d\n",n_finished );
        printf(">>en la cola :\n");
        Node* nodo = q->p_pointer->head;
        while (nodo) {
          printf(">>>>>>%s,%d,%d\n",nodo->value->name, nodo->value->time_left, nodo->value->time_left_burst);
          nodo = nodo->next;
        }
        printf("***********************************************************\n");
        printf("Durante de periodo\n");
        printf("------------------\n");


        //para los que esperan o estan ready
        for (size_t i = 0; i < n_processes; i++) {
          if (q->all_p[i].actual == READY) {
            q->all_p[i].waiting_time++;
            if (q->all_p[i].actual_burst<1) {
              q->all_p[i].response_time++;
            }
          }

          if (q->all_p[i].actual == WAITING) {
            q->all_p[i].waiting_time++;
            q->all_p[i].time_left_burst --;
            if (q->all_p[i].time_left_burst == 0) {
              printf("%s dejo de esperar\n",q->all_p[i].name);
              q->all_p[i].actual = READY;
              q->all_p[i].actual_burst++;
              q->all_p[i].time_left_burst= q->all_p[i].bursts[q->all_p[i].actual_burst];
              if (q->p_pointer->size == 0) {
                ll_append(q->p_pointer,&q->all_p[i]);
              }
              else{
                printf("[en cola despues de wait] en la cabeza:%s\n",q->all_p[i].name );
                ll_insert(&q->all_p[i],q->p_pointer->head,q->p_pointer,0);
              }
            }
          }

          q->all_p[i].turnaround_time != 0 && q->all_p[i].actual!=FINISHED ? q->all_p[i].turnaround_time++ : q->all_p[i].turnaround_time;//si ya llegó, aumento su t.a.time
        }


        //Que pasa si se acaba el quantum
        if (left_quantum == 0) {
          printf("--------------------fin de un quantum--------------------\n");

          //Si la cola no esta vacia
          if (q->p_pointer->size>0) {
            on_cpu->interrupted++;

            // y si queda tiempo de burst
            if (on_cpu->time_left_burst>0 && on_cpu!=idle) {
              on_cpu->actual = READY;
              //on_cpu->response_time++;
              if (on_cpu!=idle) {
                ll_insert(on_cpu,q->p_pointer->head,q->p_pointer,0); //O(n)
              }
            }
            // y si se acaba el burst
            else{
              on_cpu!=idle?on_cpu->actual_burst++:on_cpu->actual_burst;
              printf("%s, mi burst actual %d\n",on_cpu->name,on_cpu->actual_burst);

              if (on_cpu->actual_burst<2*on_cpu->n_burst-1) {
                on_cpu->actual = WAITING;
                on_cpu->time_left_burst = on_cpu->bursts[on_cpu->actual_burst];
                printf("    +++    %s deberia esperar %d en burst %d    +++   \n",on_cpu->name,on_cpu->bursts[on_cpu->actual_burst],on_cpu->actual_burst);

              }
              else{
                on_cpu->actual = FINISHED;
                if (on_cpu!=idle) {
                  n_finished++;
                }
              }
            }

            if (q->p_pointer->size>0) {
              printf("////////////////////saque a %s de la cpu\n",on_cpu->name);
              on_cpu = q->p_pointer->head->value;
              if (on_cpu->actual_burst==0 & on_cpu->interrupted==0) {
                on_cpu->response_time=timer-on_cpu->time_of_arrival;
                printf("|/_ seteando response_time a %s\n",on_cpu->name);
              }
              on_cpu->chosen++;
              ll_pop(q->p_pointer);
              on_cpu->actual = RUNNING;
              //on_cpu->time_left_burst == 0 ? on_cpu->bursts[on_cpu->actual_burst] : on_cpu->time_left_burst;
              printf("    [q==0]oooo    %s deberia entrar con time_left %d en burst %d  oooo   \n",on_cpu->name,on_cpu->time_left,on_cpu->actual_burst);
            }

          }
          //Si la cola esta vacia
          else{
            //Si todavia me queda tiempo de burst
            if (on_cpu->time_left_burst>0) {
              on_cpu->interrupted++;
              printf("[q==0]Puedo seguir corriendo a %s\n",on_cpu->name);
            }
            //si no me queda burst
            else{
              on_cpu!=idle?on_cpu->actual_burst++:on_cpu->actual_burst;
              printf("%s, mi burst actual %d\n",on_cpu->name,on_cpu->actual_burst);

              //si ese no era mi ultimo burst
              if (on_cpu->actual_burst<2*on_cpu->n_burst-1) {
                on_cpu->actual = WAITING;
                on_cpu->time_left_burst = on_cpu->bursts[on_cpu->actual_burst];
                printf("    +++    %s deberia esperar %d en burst %d    +++   \n",on_cpu->name,on_cpu->bursts[on_cpu->actual_burst],on_cpu->actual_burst);

              }
              //si ese era mi ulitmo burst
              else {
                on_cpu->actual=FINISHED;
                if (on_cpu!=idle) {
                  n_finished++;
                }
              }
              on_cpu = idle;
            }//
          }
          left_quantum = quantum;
        }


        else{
          //Si queda quantum
          if (left_quantum>0) {

            //Si se me acabo el burst
            if (on_cpu->time_left_burst==0) {
              on_cpu!=idle?on_cpu->actual_burst++:on_cpu->actual_burst;
              printf("%s, mi burst actual %d\n",on_cpu->name,on_cpu->actual_burst);

              //Si no era mi ultimo burst
              if (on_cpu->actual_burst<2*on_cpu->n_burst-1) {
                on_cpu->actual = WAITING;
                on_cpu->time_left_burst = on_cpu->bursts[on_cpu->actual_burst];
                printf("    +++    %s deberia esperar %d en burst %d    +++   \n",on_cpu->name,on_cpu->bursts[on_cpu->actual_burst],on_cpu->actual_burst);

              }

              //Si era mi último burst
              else{
                on_cpu->actual=FINISHED;
                if (on_cpu!=idle) {
                  n_finished++;
                }
              }

              //Puedo poner a alguien de la lista
              if (q->p_pointer->size>0) {
                printf("////////////////////saque a %s de la cpu\n",on_cpu->name);
                on_cpu = q->p_pointer->head->value;
                if (on_cpu->actual_burst<1 & on_cpu->interrupted==0) {
                  on_cpu->response_time=timer - on_cpu->time_of_arrival;
                  printf("|/_ seteando response_time a %s\n",on_cpu->name);
                }
                on_cpu->chosen++;
                ll_pop(q->p_pointer);
                on_cpu->actual = RUNNING;
                //on_cpu->time_left_burst == 0 ? on_cpu->bursts[on_cpu->actual_burst] : on_cpu->time_left_burst;
                printf("    [q>0]oooo    %s deberia entrar con time_left %d en burst %d  oooo   \n",on_cpu->name,on_cpu->time_left,on_cpu->actual_burst);

              }

              //No hay nadie en la lista
              else{
                printf("jgfcjyfcjyfxkyrfxkskyrsz,ytx,ucluyvulyhvlhb\n");
                on_cpu = idle;
              }
            }

            //Si me queda burst
            else{
              printf("Puedo seguir corriendo %s\n",on_cpu->name);

            }
          }
        }
        //Si solo esta corriendo el idle que pase el siguiente
        if(on_cpu == idle && q->p_pointer->size>0 ){
          //q->p_pointer->head->value->actual=RUNNING;
          printf("////////////////////saque a %s de la cpu\n",on_cpu->name);
          on_cpu = q->p_pointer->head->value;
          on_cpu->chosen++;
          ll_pop(q->p_pointer);
          on_cpu->actual = RUNNING;
          //on_cpu->time_left_burst == 0 ? on_cpu->bursts[on_cpu->actual_burst] : on_cpu->time_left_burst;
        }

        for (size_t i = 0; i < n_processes; i++) {
          //Chequear si llegó algún proceso
          if (timer == q->all_p[i].time_of_arrival && arrival_ptr<n_processes) {
            q->all_p[i].actual = READY;
            q->all_p[i].time_left_burst = q->all_p[i].bursts[q->all_p[i].actual_burst];
            q->all_p[i].turnaround_time++;
            printf("   !!!   llegue %s, mi primer burst es de: %d   !!!   \n",q->all_p[i].name, q->all_p[i].time_left_burst);
            q->p_pointer->size == 0 ?
              ll_append(q->p_pointer,&q->all_p[i]) :
              ll_insert(&q->all_p[i],q->p_pointer->head,q->p_pointer,0);
            arrival_ptr +=1 ;
          }
        }


        timer ++;
        left_quantum --;
        if (on_cpu != idle) {
          on_cpu->time_left_burst --;
          on_cpu->time_left--;

        }
        else{
          on_cpu->time_left_burst = 255;
          on_cpu->time_left = 255;
        }

        if (timer==50) {
          n_finished=10;
          printf("error: forced\n");
        }

      }



/*--------------------------Version non-preemptive----------------------------*/
      else if (!strcmp(version, "np")) {


        printf("timer:%d, %s,le queda %d\n",timer,on_cpu->name, on_cpu->time_left_burst);
        for (size_t i = 0; i < n_processes; i++){
          if (q->all_p[i].actual==FINISHED) {
            printf("-----%s ya termino con turnaround_time:%d\n",q->all_p[i].name,q->all_p[i].turnaround_time);
          }
          printf("nombre:%s,estado:%d, time_left:%d\n",q->all_p[i].name,q->all_p[i].actual,q->all_p[i].time_left);
        }

        //Chequear si llegó algún proceso
        for (size_t i = 0; i < n_processes; i++) {
          if (timer == q->all_p[i].time_of_arrival && arrival_ptr<n_processes) {
            q->all_p[i].actual = READY;
            q->all_p[i].time_left_burst = q->all_p[i].bursts[q->all_p[i].actual_burst];
            q->all_p[i].turnaround_time++;
            printf("   !!!   llegue %s, mi primer burst es de: %d   !!!   \n",q->all_p[i].name, q->all_p[i].time_left_burst);
            q->p_pointer->size == 0 ?
              ll_append(q->p_pointer,&q->all_p[i]) :
              ll_insert(&q->all_p[i],q->p_pointer->head,q->p_pointer,0);
            arrival_ptr +=1 ;

          }
        }

        //para los que esperan o estan ready
        for (size_t i = 0; i < n_processes; i++) {
          if (q->all_p[i].actual == READY) {
            //if (q->all_p[i].actual_burst>1) {
            //  q->all_p[i].waiting_time++;
            //}
            q->all_p[i].waiting_time++;

            if (q->all_p[i].actual_burst<1) {
              q->all_p[i].response_time++;
            }
            printf("%s, llevo listo %d\n", q->all_p[i].name,q->all_p[i].response_time);

          }
          if (q->all_p[i].actual == WAITING) {
            q->all_p[i].time_left_burst --;
            q->all_p[i].waiting_time++;

            if (q->all_p[i].time_left_burst == 0) {
              q->all_p[i].actual = READY;
              q->all_p[i].actual_burst++;
              q->all_p[i].time_left_burst= q->all_p[i].bursts[q->all_p[i].actual_burst];
              if (q->p_pointer->size == 0) {
                ll_append(q->p_pointer,&q->all_p[i]);
              }
              else{
                ll_insert(&q->all_p[i],q->p_pointer->head,q->p_pointer,0);
              }

            }
          }

          q->all_p[i].turnaround_time != 0 && q->all_p[i].actual!=FINISHED ? q->all_p[i].turnaround_time++ : q->all_p[i].turnaround_time;//si ya llegó, aumento su t.a.time
        }


        if (on_cpu->time_left_burst==0) {
          on_cpu->actual_burst++;
          if (on_cpu->actual_burst<2*on_cpu->n_burst-1) {
            on_cpu->actual = WAITING;
            on_cpu->time_left_burst = on_cpu->bursts[on_cpu->actual_burst];
            printf("    +++    %s deberia esperar %d en burst %d    +++   \n",on_cpu->name,on_cpu->bursts[on_cpu->actual_burst],on_cpu->actual_burst);
          }
          else {
            on_cpu->actual=FINISHED;
            if (on_cpu!=idle) {
              n_finished++;
            }
            printf("termino %s\n",on_cpu->name);
          }
          if (q->p_pointer->size>0) {
            on_cpu = q->p_pointer->head->value;
            on_cpu->chosen++;
            ll_pop(q->p_pointer);
            on_cpu->actual = RUNNING;
            printf("    oooo    %s deberia entrar con time_left %d en burst %d  oooo   \n",on_cpu->name,on_cpu->time_left,on_cpu->actual_burst);
            on_cpu->time_left_burst == 0 ? on_cpu->bursts[on_cpu->actual_burst] : on_cpu->time_left_burst;

          }
          else{
            on_cpu = idle;
            printf("Puse el idle (2)\n");

          }

        }
        //Si solo esta corriendo el idle que pase el siguiente
        if(on_cpu == idle && q->p_pointer->size>0 ){
          on_cpu = q->p_pointer->head->value;
          on_cpu->chosen++;
          ll_pop(q->p_pointer);
          on_cpu->actual = RUNNING;
          on_cpu->time_left_burst == 0 ? on_cpu->bursts[on_cpu->actual_burst] : on_cpu->time_left_burst;
        }

        timer += 1;
        if (on_cpu != idle) {
          on_cpu->time_left_burst --;
          on_cpu->time_left--;
        }
        else{
          on_cpu->time_left_burst = 255;
        }
      }




/*----------------------------Control de errores------------------------------*/
      else{
        printf("Something went wrong for version input\n");
      }
    }
    printf("tiempo final%d\n",timer );

    FILE* outputfile;
    outputfile = fopen (outputname, "w+");

    for (size_t i = 0; i < n_processes; i++) {
      printf("%s,%d,%d,%d\n", q->all_p[i].name,q->all_p[i].turnaround_time-1,q->all_p[i].waiting_time,q->all_p[i].response_time);
      fprintf(outputfile, "%s,%d,%d,%d,%d,%d\n", q->all_p[i].name, q->all_p[i].chosen, q->all_p[i].interrupted, q->all_p[i].turnaround_time-2, q->all_p[i].response_time-1, q->all_p[i].waiting_time-1);
    }
    fclose(outputfile);


    for (size_t i = 0; i < n_processes; i++) {
      process_destroy(all_p[i]);
    }
    process_destroy(idle);
    queu_destroy(q,n_processes);










  }

}


//TODO:No funciona p
