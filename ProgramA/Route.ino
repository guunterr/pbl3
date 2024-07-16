#include <Wire.h>
#include <ZumoShieldN.h>

typedef struct{
  int array[12];
  int index;
} Queue;

int push(Queue *q, int item);
bool isEmpty(Queue* q);
int pop(Queue* q);


int push(Queue *q, int item){
  if (q->index > 11){
    Serial.println("Reading from empty queue!");
    return 0;
  } else{
    q->array[q->index] = item;
    q->index++;
    return 1;
  }
}

bool isEmpty(Queue* q){
  return (q->index == 0);
}

int pop(Queue* q){
  if (q->index == 0){
    return -1;
  } else{
    q->index--;
    int item = q->array[q->index];
    return item;
  }
}

int graph[12][12] = {
// 0,1,2,3,4,5,6,7,8,9,a,b
  {1,1,0,0,1,0,0,0,0,0,0,0}, //0
  {1,1,1,0,0,0,0,0,0,0,0,0}, //1
  {0,1,1,1,0,0,0,0,0,0,0,0}, //2
  {0,0,1,1,0,0,0,1,0,0,0,0}, //3
  {1,0,0,0,1,1,0,0,1,0,0,0}, //4
  {0,0,0,0,1,1,1,0,0,0,0,0}, //5
  {0,0,0,0,0,1,1,1,0,0,1,0}, //6
  {0,0,0,1,0,0,1,1,0,0,0,0}, //7
  {0,0,0,0,1,0,0,0,1,1,0,0}, //8
  {0,0,0,0,0,0,0,0,1,1,0,0}, //9
  {0,0,0,0,0,0,1,0,0,0,1,1}, //a
  {0,0,0,0,0,0,0,0,0,0,1,1}, //b
// 0,1,2,3,4,5,6,7,8,9,a,b
};

void bfs(int start, int end){
  int predecessor[12];
  bool visited[12] = {false,false,false,false,false,false,false,false,false,false,false,false};
  Queue unvisited = {{0}, 0};
  push(&unvisited,start);
  while (!isEmpty(&unvisited)){
    int current = pop(&unvisited);
    if(!visited[current]){
      visited[current] = true;
      // if (current == end) return;
      for (int i = 0; i < 12;i++){
        if (graph[current][i] == 1 && !visited[i]){
          push(&unvisited,i);
          predecessor[i] = current;
        } 
      }
    }
  }
}