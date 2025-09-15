#include "vector.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>


typedef uintptr_t Obj;

struct Cora;

/* typedef void (*basicBlock) (struct Cora *co, Obj *R); */
typedef void (*basicBlock) (struct Cora *co, int label, Obj *R);

struct frame {
	Obj *frame;
	int label;
	basicBlock fn;
};

const int INIT_STACK_SIZE = 256;

struct stackAllocator {
	// cache fields for fast allocation path.
	Obj *base;
	int start;
	int end;

	int pos;
	vector(Obj*) data;
};

static void
stackAllocatorInit(struct stackAllocator *alloc) {
	alloc->pos = 0;
	vecInit(&alloc->data, 8);
	Obj *fixSized = (Obj*)malloc(sizeof(Obj) * INIT_STACK_SIZE);
	vecAppend(&alloc->data, fixSized);

	alloc->base = fixSized;
	alloc->start = 0;
	alloc->end = INIT_STACK_SIZE;
}

static Obj*
stackAllocSlowPath(struct stackAllocator *sa, int n) {
	assert(false);
}
	/* fixSizedSegStack* s = vectorGet(&sa->data, sa->curr); */
	/* if (s->pos + n > INIT_STACK_SIZE) { */
	/* 	if (sa->curr < vecLen(&sa->data)) { */
	/* 		sa->curr++; */
	/* 		s = vectorGet(&sa->data, sa->curr); */
	/* 	} else { */
	/* 		s = (struct fixSizedSegStack*)malloc(sizeof(struct fixSizedSegStack)); */
	/* 		vecAppend(&sa->data, s); */
	/* 		sa->curr++; */
	/* 	} */

	/* } */
	/* Obj* ret = s->stack + s->pos; */
	/* s->pos += n; */
	/* return ret; */
/* } */

/* static inline void */
/* stackAllocUndo(struct stackAllocator *sa, struct stackPos r) { */
/* } */

struct objStack {
	struct stackAllocator alloc;
	vector(int) history;
};

static void objStackInit(struct objStack *os) {
	stackAllocatorInit(&os->alloc);
	vecInit(&os->history, 32);
}

static inline Obj*
stackAlloc(struct objStack *os, int n) {
	vecAppend(&os->history, os->alloc.pos);
	// fast path
	if (os->alloc.pos + n < os->alloc.end) {
		Obj *ret = &os->alloc.base[os->alloc.pos - os->alloc.start];
		os->alloc.pos += n;
		return ret;
	}
	return stackAllocSlowPath(&os->alloc, n);
}

void
stackUndo(struct objStack *os) {
	int pos = vecPop(&os->history);
	if (pos >= os->alloc.start) {
		os->alloc.pos = pos;
		return;
	}
	// slow path stackAllocUndo?
	assert(false);
}

struct Cora {
	vector(struct frame) callstack;
	struct frame ctx;
	struct objStack stk;
	Obj res;
};

static struct Cora *
coraNew() {
	struct Cora *co = malloc(sizeof(struct Cora));
	vecInit(&co->callstack, 64);
	objStackInit(&co->stk);
	return co;
}

void
trampoline(struct Cora *co) {
	while(co->ctx.fn != NULL) {
		/* co->ctx.fn(co, co->ctx.frame); */
		co->ctx.fn(co, co->ctx.label, co->ctx.frame);
	}
}

struct scmNative {
	/* scmHead head; */
	/* struct pcState code; */
	// required is the argument number of the nativeFunc.
	int required;
	int nframe;
	basicBlock fn;
	// captured is the size of the data, it's immutable after makeNative.
	/* int captured; */
	/* Obj data[]; */
};

#define ptr(x) ((void*)((x)))

Obj
makeNative(basicBlock fn, int required, int nframe) {
	/* int sz = sizeof(struct scmNative) + captured * sizeof(Obj); */
	/* struct scmNative *clo = newObj(scmHeadNative, sz); */
	struct scmNative *clo = malloc(sizeof(struct scmNative));
	clo->fn = fn;
	/* clo->code.label = label; */
	clo->required = required;
	clo->nframe = nframe;
	/* clo->captured = captured; */
	/* if (captured > 0) { */
	/* 	va_list ap; */
	/* 	va_start(ap, captured); */
	/* 	for (int i = 0; i < captured; i++) { */
	/* 		clo->data[i] = va_arg(ap, Obj); */
	/* 	} */
	/* 	va_end(ap); */
	/* } */

	return (Obj) clo;
	/* return ((Obj) (&clo->head) | TAG_PTR); */
}

struct scmNative *
mustNative(Obj o) {
	struct scmNative *native = ptr(o);
	/* assert(native->head.type == scmHeadNative); */
	return native;
}

void
coraCall(struct Cora *co, Obj f, int nargs, ...) {
	struct scmNative* fn = mustNative(f);
	if (fn->required != nargs) {
		// curry or partial, goto dispatch function
		return;
	}
	int nframe = fn->nframe;
	assert(nframe >= nargs);
	// prepare for callee's frame
	Obj *frame = stackAlloc(&co->stk, nframe);
	// prepare for callee's arguments
	frame[0] = f;
	if (nargs > 0) {
		va_list ap;
		va_start(ap, nargs);
		for (int i = 1; i <= nargs; i++) {
			frame[i] = va_arg(ap, Obj);
		}
		va_end(ap);
	}
	// set the new ctx
	struct frame curr = {
		.fn = fn->fn,
		.label = 0,
		.frame = frame,
	};
	co->ctx = curr;
	return;
}

void
coraReturn(struct Cora *co, Obj val) {
	// pop stack
	stackUndo(&co->stk);
	// set return value
	co->res = val;
	// recover continuation
	co->ctx = vecPop(&co->callstack);
	return;
}

#define MAKE_NUMBER(v) ((Obj) ((intptr_t) (v) << 1))


static void clofun1(struct Cora* co, int label, Obj *R);
static void clofun0(struct Cora* co, int label, Obj *R);
/* static void clofun1(struct Cora* co, Obj *R); */
/* static void clofun0(struct Cora* co, Obj *R); */

static __thread Obj* __symbolTable;

static void clofun1(struct Cora* co, int label, Obj *R) {
	switch (label) {
	case 0:
		{
			Obj x4318524071 = __symbolTable[0] = makeNative(clofun0, 1, 2);
			stackUndo(&co->stk);
			coraCall(co, __symbolTable[0], 1, MAKE_NUMBER(40));
			return;
		}
	}
}


#define fixnum(x) ((intptr_t)(x)>>1)

#define PRIM_LT(x, y) (fixnum(x) < fixnum(y) ? True : False)
#define PRIM_SUB(x, y)    ((x) - (y))
#define PRIM_ADD(x, y)    ((x) + (y))


#define TAG_BOOLEAN 0xd
#define TAG_SHIFT 3
const Obj True = ((1 << (TAG_SHIFT + 1)) | TAG_BOOLEAN);
const Obj False = ((2 << (TAG_SHIFT + 1)) | TAG_BOOLEAN);

static void clofun0(struct Cora* co, int label, Obj *R) {
	switch (label) {
	case 0:
		{
			Obj n = R[1];
			Obj x4318576263 = PRIM_LT(n, MAKE_NUMBER(2));
			if (True == x4318576263) {
				coraReturn(co, n);
				return;
			} else {
				Obj x4318523495 = PRIM_SUB(n, MAKE_NUMBER(1));
				R[1] = n;
				struct frame tmp = {
					.fn = clofun0,
					.label = 1,
					.frame = R,
				};
				vecAppend(&co->callstack, tmp);

				/* coraCall(co, __symbolTable[0], 1, x4318523495); */
				Obj* frame = stackAlloc(&co->stk, 2);
				frame[0] = __symbolTable[0];
				frame[1] = x4318523495;
				struct frame curr = {
					.fn = clofun0,
					.label = 0,
					.frame = frame,
				};
				co->ctx = curr;

				return;
			}
		}
	case 1:
		{
			Obj x4318523527= co->res;
			Obj n = R[1];
			Obj x4318523751 = PRIM_SUB(n, MAKE_NUMBER(2));
			R[1] = x4318523527;
			struct frame tmp1 = {
				.fn = clofun0,
				.label = 2,
				.frame = R,
			};

			vecAppend(&co->callstack, tmp1);

			/* coraCall(co, __symbolTable[0], 1, x4318523751); */
			Obj *frame = stackAlloc(&co->stk, 2);
			frame[0] = __symbolTable[0];
			frame[1] = x4318523751;
			struct frame curr = {
				.fn = clofun0,
				.label = 0,
				.frame = frame,
			};
			co->ctx = curr;

			return;
		}
	case 2:
		{
			Obj x4318523815= co->res;
			Obj x4318523527 = R[1];
			Obj x4318524039 = PRIM_ADD(x4318523527, x4318523815);
			coraReturn(co, x4318524039);
			return;
		}
	}
}

int main() {
	struct Cora *co = coraNew();

	__symbolTable = (Obj*)malloc(sizeof(Obj) * 1);
	/* __symbolTable[0] = intern("fib"); */

	struct frame tmp2 = {.fn = NULL};
	vecAppend(&co->callstack, tmp2);
	Obj *frame = stackAlloc(&co->stk, 0);
	struct frame tmp3 = {
		.fn = clofun1,
		.label = 0,
		.frame = frame,
	};
	co->ctx = tmp3;
	trampoline(co);

	printf("result == %ld\n", fixnum(co->res));
	return 0;
}
