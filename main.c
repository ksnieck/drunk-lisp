#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define t_void 0
#define t_list 1
#define t_symbol 2
#define t_integer 3
#define t_real 4
#define t_string 5

typedef uint64_t typeid;

typedef struct type {
  uint64_t id;
  void* data;
} type;

typedef struct list {
  struct list* next;
  struct list* prev;
  struct type type;
} list;

list* list_make(list* prev, list* next, typeid t, void* data) {
  printf("list_make(): ");
  list* l = malloc(sizeof(list));
  l->prev = prev;
  l->next = next;
  l->type.id = t;
  l->type.data = data;
  printf("%p\n", l);
  return l;
}

typedef struct string {
  uint32_t l;
  char* c;
} string;

string* string_make(char* b, uint32_t l) {
  printf("string_make(%.*s, %d)=", l, b, l);
  string* s = malloc(sizeof(string));
  s->c = memcpy(malloc(sizeof(char)*l), b, l);
  s->l = l;
  printf("%p\n", s);
  return s;
}

typedef double real;
real* real_make(real f) {
  real* p;
  p = malloc(sizeof(real));
  *p = f;
  return p;
}

typedef int64_t integer;
integer* integer_make(integer i) {
  integer* p;
  p = malloc(sizeof(integer));
  *p = i;
  return p;
}

char input() {
  char a = getc(stdin);
  return a;
}

void output(char c) {
  printf("%c", c);
}
void outputs(char* s) {
  printf("%s", s);
}
void outputb(char* s, int l) {
  printf("%.*s", l, s);
}

list* parse_atom(char* buf, uint32_t len) {
  integer i;
  float r;
  if (sscanf(buf, "%lld", &i) != 0)
    return list_make(NULL, NULL, t_integer, integer_make(i));
  else if(sscanf(buf, "%f", &r) != 0)
    return list_make(NULL, NULL, t_real, real_make(r));
  else { // MUST BE A SYMBOL!
    void* s = string_make(buf, len);
    return list_make(NULL, NULL, t_symbol, s);
  }
}

list* parse_string() {
  char a;
  int bufl = 256;
  char buf[bufl+1];
  int bufi = 0;
  char* d;
  while (1) {
    a = input();
    if (a == '"' && buf[bufi] != '\\')
      return list_make(NULL, NULL, t_string, string_make(buf, bufi));
    if (bufi>=bufl) {
      buf[bufl] = '\0';
      printf("PANIC: reading input atom with length > %d characters:\n'%s'\n", bufl, buf);
      exit(-1);
    }
    buf[bufi] = a;
    bufi++;   
  }
}
    
list* read();
list* read() {
  list* h = list_make(NULL, NULL, t_list, NULL);
  list* l = h;
  char a;
  int bufl =256;
  char buf[bufl+1];
  int bufi = 0;
  while (1) {
    a = input();
    switch (a) {
    case '(':
      l->next = list_make(NULL, NULL, t_list, read());
      l = l->next;
      break;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case ')':
      if (bufi > 0) {
	buf[bufi] = '\0';
	l->next = parse_atom(buf, bufi);
	l->next->prev = l;
	l = l->next;
	bufi = 0;
      }
      if (a == ')') {
	return h;
      }
      break;
    case '"':
      l->next = parse_string();
      l->next->prev = l;
      l = l->next;
      break;
    default:
      if (bufi>=bufl) {
	buf[bufl] = '\0';
	printf("PANIC: reading input atom with length > 256 characters:\n'%s'\n", buf);
	exit(-1);
      }
      buf[bufi] = a;
      bufi++;
    }
  }
}

void print(list* l) {
  char* buf;
  output('(');
  l = l->next; // list head is a list type with null data
  while (1) {
    if (l == NULL) {
      output(')');
      return;
    }
    switch (l->type.id) {
    case t_list:
      print((list*)l->type.data);
      break;
    case t_symbol:
      outputb(((string*)l->type.data)->c, ((string*)l->type.data)->l);
      break;
    case t_string:
      output('"');
      outputb(((string*)l->type.data)->c, ((string*)l->type.data)->l);	
      output('"');
      break;
    case t_integer:
      asprintf(&buf, "%lld", *((integer*)l->type.data));
      outputs(buf);
      free(buf);
      break;
    case t_real:
      asprintf(&buf, "%lf", *(real*)(l->type.data));
      outputs(buf);
      free(buf);
      break;
    default:
      outputs("VOID");
    }
    l = l->next;
    if (l == NULL) {
      output(')');
      return;
    }
    output(' ');
  }
}

void print_test() {
  list* l;
  list* h;
  h = list_make(NULL, NULL, t_list, NULL);
  l = h;
  l->next = list_make(NULL, NULL, t_symbol, string_make("+", 1));
  l->next->prev = l;
  l = l->next;
  l->next = list_make(NULL, NULL, t_integer, integer_make(3));
  l->next->prev = l;
  l = l->next;
  l->next = list_make(NULL, NULL, t_integer, integer_make(4));
  l->next->prev = l;
  print(h);
}

typedef struct env {
  struct env* parent;
  list* procedures;
} env;

env* env_make(env* parent, list* procedures) {
  env* e = malloc(sizeof(env));
  e->parent=parent;
  e->procedures=procedures;
  return e;
}

p

list* eval(list*);
list* eval(list* h, env* parent) {
  list* l = h;
  list* r = list_make(NULL, NULL, t_list, NULL);

  while (l->next != NULL) {
    // traverse the whole fackin list
    switch (l->type.id) {
    case t_void:
      printf("eval() found a t_void?!\n");
      break;
    case t_list: // well clearly we gotta EVAL dis lis
      l->type.data = eval(l->type.data);
      break;
    case t_symbol: // fun part
      


int main(int argc, char** argv) {
  int depth = 0;
  char a;
  list* l;
  printf("(");
  l = read();
  // eval later
  print(l);
  output('\n');
}
