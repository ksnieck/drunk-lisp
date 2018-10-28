// disclaimer.... i'm drunk
// i know this isn't lisp
// i was stupid and implemented it on a base of lists and not pairs
// so go fuck yourself i can do whatever i want

/* data structure format:

- list -> eval -> type
- eval looks up the symbol in the procedure table
- eval executes the found procedure and passes it the original list
- the procedure is then free to take the arguments as is, or eval them. The typical pattern would be to eval each argument and perform an operation with their results.
- Do we still need a mechanism for quoting?
*/


#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define debug(...) fprintf (stderr, __VA_ARGS__)
#define error(...) fprintf (stderr, __VA_ARGS__); exit(-1);

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

type type_make(typeid id, void* data) {
  type t = {id, data};
  return t;
}

typedef struct list {
  struct list* next;
  struct list* prev;
  struct type type;
} list;

list* list_make(list* prev, list* next, typeid t, void* data) {
  list* l = malloc(sizeof(list)); // yolo
  l->prev = prev;
  l->next = next;
  l->type = type_make(t, data);
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

int string_eq(string s1, string s2) { // returns 0 if they are equal
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

#define input_EOF EOF
#define input_ERR (-2)

int input() {
  int a = getc(stdin);
  if (a == EOF && feof(stdin)) {
    clearerr(stdin);
    return input_EOF;
  }
  if (a == EOF && ferror(stdin)) {
    clearerr(stdin);
    return input_ERR;
  }
  return a;
}

FILE* fd_out;

void output(char c) {
  fprintf(fd_out, "%c", c);
}
void outputs(char* s) {
  fprintf(fd_out, "%s", s);
}
void outputb(char* s, int l) {
  fprintf(fd_out, "%.*s", l, s);
}
void outputp(string* s) {
  fprintf(fd_out, "%.*s", s->l, s->c);
}
void outputf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(fd_out, fmt, args);
  va_end(args);
}

type parse_atom(char* buf, uint32_t len) {
  //  integer i;
  float r;
  /*  if (sscanf(buf, "%lld", &i) != 0)
    return list_make(NULL, NULL, t_integer, integer_make(i));
    else*/
  if(sscanf(buf, "%f", &r) != 0)
    return type_make(t_real, real_make(r));
  else { // MUST BE A SYMBOL!
    void* s = string_make(buf, len);
    return type_make(t_symbol, s);
  }
}

type parse_string() {
  int a;
  int bufl = 256;
  char buf[bufl+1];
  int bufi = 0;
  while (1) {
    a = input();
    if (a <= input_EOF) {
      error("PANIC: EOF (-1) or read error (-2) encountered during parse_string() (%i)", a);
    }
    if (a == '"' && buf[bufi] != '\\')
      return type_make(t_string, string_make(buf, bufi));
    if (bufi>=bufl) {
      buf[bufl] = '\0';
      error("PANIC: reading input atom with length > %d characters:\n'%s'\n", bufl, buf);
    }
    buf[bufi] = a;
    bufi++;   
  }
}
    
list* read();
list* read() {
  int a;
  list* l = list_make(NULL, NULL, t_list, NULL);
  list* h = l;
  int bufl = 256;
  char buf[bufl+1];
  int bufi = 0;
  while (1) {
    a = input();
    switch (a) {
    case input_ERR:
      error("PANIC: encountered input stream error in read() (%i)", a);
    case '(':
      l->type.id = t_list;
      l->type.data = read();
      break;
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case ')':
    case input_EOF:
      if (bufi > 0) {
	buf[bufi] = '\0';
	l->type = parse_atom(buf, bufi);
	l->next = list_make(l, NULL, t_list, NULL);
	l = l->next;
	bufi = 0;
      }
      if (a == ')' || a == input_EOF) {
	return h;
      }
      break;
    case '"':
      l->type = parse_string();
      l->next = list_make(l, NULL, t_list, NULL);
      l = l->next;
      break;
    default:
      if (bufi>=bufl) {
	buf[bufl] = '\0';
	error("PANIC: reading input atom with length > 256 characters:\n'%s'\n", buf);
      }
      buf[bufi] = a;
      bufi++;
    }
  }
}

void print(list* l) {
  output('(');
  while (1) {
    if (l == NULL) {
      output(')');
      return;
    }
    switch (l->type.id) {
    case t_list:
      if (l->type.data == NULL) {
	output(')');
	return;
      }
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
      outputf("%lld", *((integer*)l->type.data));
      break;*/
    case t_real:
      outputf("%lf", *(real*)(l->type.data));
      break;
    default:
      outputf("t_unknown(%d,%p)", l->type.id, l->type.data);
    }
    l = l->next;
    if (l == NULL || (l->type.id == t_list && l->type.data == NULL)) {
      output(')');
      return;
    }
    output(' ');
  }
}

void print_test() {
  list* l = list_make(NULL, NULL, t_symbol, string_make("+", 1));;
  list* h = l;
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
  type (*call)(list* args);
} procedure;

procedure* procedure_make(string* name, string* help, type (*call)(list* args)) {
  procedure* p = malloc(sizeof(procedure));
  memcpy(&p->name, name, sizeof(string));
  free(name);
  memcpy(&p->help, help, sizeof(string)); free(help); // haha free help
  p->call=call;
  return p;
}

type eval(list* h);

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

void procedure_add(env* e, char* name, char* help, type (*call)(list* args)) {
  // take some e, add a procedural beat and tada you have a party
  // adds given basic procedure to global environment
  e->procedures = list_make(NULL, e->procedures, t_procedure,
			    procedure_make(string_cstr(name), string_cstr(help),
					   call));
}

procedure* env_find(env* e, string name) {
  list* l = e->procedures;
  //print(l);debug("\n");
  string ln;
  while (1) {
    if (l->type.id != t_procedure) {
      debug("WARNING: env_find() found a non-procedure (%llu) in an env, skipping\n",
	     l->type.id);
    } else if (string_eq(name, ((procedure*)l->type.data)->name)==0) {
      return l->type.data;
    }
    if (!l->next && e->parent) {
      env_find(e->parent, name);
    } else if (l->next) {
      l = l->next;
    } else {
      error("PANIC: procedure '%.*s' not found.\n", name.l, name.c);
    }
  }
}

type basic_add(list* a) { // after eval a better be all int / float
  type t;
  real acc = 0;
  if (a->next) {
    a = a->next;
  } else {
    a=list_make(a, NULL, t_real, real_make(0));
  }
  while (1) {
    //    print(a);debug("\n");
    t = eval(a);
    if (t.id == t_real) {
      acc += *((real*)t.data);
    }
    // TODO: list_free(t)
    if (!a->next) break;
    a = a->next;
    if (a->type.id == t_list && a->type.data == NULL) break;
  }
  return type_make(t_real, real_make(acc)); // this one eats the whole list
}
// other procedures could eat only some of the argument list and then release the rest of it

list* arg_eval(list* a, type default_type) {
  // evals all elements of cdr of a that are lists
  a = a->next;
  if (a == NULL || (a->type.id == t_list && a->type.data == NULL)) {
    a = list_make(NULL, NULL, t_void, NULL);
    a->type = default_type;
    return a;
  }
  list* l = a;
  while (l != NULL) {
    if (l->type.id == t_list) {
      l->type = eval(l->type.data);
    }
    l = l->next;
  }
  return a;
}

type basic_subtract(list* a) {
  print(a); debug("\n");
  a = arg_eval(a, (type){t_real, real_make(-0)});
  print(a); debug("\n");
  if (a->type.id != t_real) {
    error("PANIC: don't know what to do with this type that's not a number"
	  "... need some kinda exception thrower\n");
  }
  if (a->next == NULL || (a->next->type.id == t_list && a->next->type.data == NULL)) {
    return type_make(t_real, real_make(-*((real*)a->type.data)));
  } else {
    real c = *((real*)basic_add(list_make(NULL, a->next, t_symbol,
					  string_cstr("+"))).data);
    return type_make(t_real, real_make(*((real*)a->type.data)-c));
  }
}

void env_basic() {
  // init environment
  environment = env_make(NULL, list_make(NULL, NULL, t_list, NULL));
  // add base procedures to 'environment'
  procedure_add(environment, "+", "accumulate 'real' arguments", basic_add);
  procedure_add(environment, "-",
		"subtract cdr from car, or return negative of car if cdr is null ",
		basic_subtract);
}

type eval(list* l) {
  if (!l) // hopefully this only happens when we inside a null list
    return type_make(t_void, NULL);
  type a;
  switch (l->type.id) {
  case t_list: // well clearly we gotta EVAL dis lis
    l->type = eval(l->type.data);
    a = eval(l);
    break;
  case t_symbol: // fun part
    a = env_find(environment, *(string*)l->type.data)->call(l);
    // ^^ dats the magic ^^
    break;
  default:
    a = l->type;
    break;
  }
  return a;
}

int main(int argc, char** argv) {
  fd_out = stderr; // to allow debugging with print()
  int depth = 0;
  char a;
  list* l;
  env_basic();
  l = read();
  type t = eval(l);
  l = list_make(NULL, NULL, t.id, t.data);
  fd_out = stdout; // back to stdout so tests work
  print(l);
  output('\n');
}
