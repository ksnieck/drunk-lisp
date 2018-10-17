#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// WE GOTTA HAVE TYPES BECAUSE YOU GOTTA RESPECT YOUR UNDERLYING LANGAUGE
#define t_void 0
#define t_list 1
#define t_symbol 2
#define t_procedure 3 // lol
#define t_macro 4 // rofl
//#define t_integer 5 NOPE
#define t_real 6
#define t_string 7


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
  list* l = malloc(sizeof(list)); // yolo
  l->prev = prev;
  l->next = next;
  l->type.id = t;
  l->type.data = data;
  return l;
}

typedef struct string {
  uint32_t l;
  char* c;
} string;

string* string_make(char* b, uint32_t l) {
  string* s = malloc(sizeof(string));
  s->c = memcpy(malloc(sizeof(char)*l), b, l);
  s->l = l;
  return s;
}

string* string_cstr(char* s) {
  uint32_t i;
  for (i=0;s[i]!='\0';i++);
  return string_make(s, i);
}

int string_eq(string s1, string s2) {
  if (s1.l != s2.l) return -1; // not even the same length!
  for (uint32_t i=0;i<s1.l;i++) if (s1.c[i] != s2.c[i]) return -1;
  return 0;
}     

typedef double real;
real* real_make(real f) {
  real* p;
  p = malloc(sizeof(real));
  *p = f;
  return p;
}

/*typedef int64_t integer;
integer* integer_make(integer i) {
  integer* p;
  p = malloc(sizeof(integer));
  *p = i;
  return p;
  }*/

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
  //  integer i;
  float r;
  /*  if (sscanf(buf, "%lld", &i) != 0)
    return list_make(NULL, NULL, t_integer, integer_make(i));
    else*/
  if(sscanf(buf, "%f", &r) != 0)
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
  l = l->next; // list head is a list type with null data (probably a bad idea)
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
      /*    case t_integer:
      asprintf(&buf, "%lld", *((integer*)l->type.data));
      outputs(buf);
      free(buf);
      break;*/
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
  l->next = list_make(NULL, NULL, t_real, real_make(3));
  l->next->prev = l;
  l = l->next;
  l->next = list_make(NULL, NULL, t_real, real_make(4));
  l->next->prev = l;
  print(h);
}


typedef struct procedure {
  string name;
  string help; // haha yea right
  list* (*call)(list* args);
} procedure;

procedure* procedure_make(string* name, string* help, list* (*call)(list* args)) {
  procedure* p = malloc(sizeof(procedure));
  memcpy(&p->name, name, sizeof(string));
  free(name);
  memcpy(&p->help, help, sizeof(string)); free(help); // haha free help
  p->call=call;
  return p;
}

list* eval(list* h);

typedef struct env {
  struct env* parent;
  list* procedures;
} env;

env* environment; // the current environment context
// this will fuck lots of shit up but it's annoying otherwise, will fix later

env* env_make(env* parent, list* procedures) {
  env* e = malloc(sizeof(env));
  e->parent=parent;
  e->procedures=procedures;
  return e;
}

void procedure_add(char* name, char* help, list* (*call)(list* args)) {
  // take some e, add a procedural beat and tada you have a party
  // adds given basic procedure to global environment
  environment->procedures = list_make(NULL, environment->procedures, t_procedure,
				      procedure_make(string_cstr(name),
						     string_cstr(help),
						     call));
// why are these functions so fuckin easy (you know you've fucked up when you don't need CURLY BRACKETS
}

// not sure this declaration is necessary: procedure* env_find(env* e, string* name);
procedure* env_find(env* e, string name) {
  list* l = e->procedures;
  while (1) {
    if (l->type.id != t_procedure) {
      printf("PANIC: env_find() found a non-procedure (%llu) in an environment\n", l->type.id);
      exit(-1);
    }
    if (string_eq(name, ((procedure*)l->type.data)->name))
      return l->type.data;
    if (!l->next && e->parent)
      env_find(e->parent, name);
    else {
      printf("PANIC: couldn't find procedure named '%.*s' in environments\n", name.l, name.c);
      exit(-2);
    }
    l = l->next;
  }
}

list* basic_add(list* a) { // after eval a better be all int / float
  list* t;
  real acc = 0;
  do {
    t = eval(a);
    if (t->type.id != t_real) {
      printf("PANIC: argument to '+' did not eval to real");
      exit(-3);
    }
    acc += *((real*)t->type.data);
    // TODO: list_free(t)
  } while (a->next != NULL);
  return list_make(NULL, NULL, t_real, real_make(acc)); // this one eats the whole list
}
// other procedures could eat only some of the argument list and then release the rest of it

void env_basic() {
  // init environment
  environment = env_make(NULL, list_make(NULL, NULL, t_list, NULL));
  // add base procedures to 'environment'
  procedure_add("+", "accumulate 'real' arguments", basic_add);
}

list* eval(list* h) {
  list* l = h;
  list* p = l->prev;
  list* r = list_make(NULL, NULL, t_list, NULL);

  do {
    // traverse the whole fackin list
    switch (l->type.id) {
    case t_list: // well clearly we gotta EVAL dis lis
      l->type.data = eval(l->type.data);
      break;
    case t_symbol: // fun part
      p = l->prev;
      l = env_find(environment, *(string*)l->type.data)->call(l->next);
      l->prev = p;
      // ^^ dats the magic ^^
      break;
    case t_void:
      printf("eval() found a t_void?!\n");
    case t_real:
    case t_string:
    case t_macro:
    case t_procedure:
      l = l->next;
    }
    // when is eval gonna respect (quote ...) ???!?!?! TODO
  } while (l->next != NULL);
  return h;
}

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
