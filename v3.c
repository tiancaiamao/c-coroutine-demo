#include <stdio.h>


struct fibFrame {
	int n;
	int val1;
};

struct stack {
	char *ptr;
	int len;
	int cap;
};

struct cont;

struct Cora {
	struct stack stk;
	struct cont *conts;
	int len;

	int res;
};

struct cont {
	void (*fn)(struct Cora *co, int label, void* frame);
	int label;
	void *frame;
};


void trampoline(struct Cora *co) {
	while(co->len > 0) {
		struct cont* c = &co->conts[--co->len];
		c->fn(co, c->label, c->frame);
	}
}


// (defun fib (n)
//  (if (< n 2)
//      n 
//      (+ (fib (- n 1)) (fib (- n 2)))))

void fib(struct Cora *co, int label, void *ptr) {
	struct fibFrame *frame = ptr;
	switch (label) {
	case 0:
		if (frame->n < 2) {
			co->res = frame->n;
			return;
		} else {
			// save continuation
			// after fib(n - 2) return, call this cont, the label is 1
			struct cont ret = {
				.fn = fib,
				.label = 1,
				.frame = frame,
			};
			co->conts[co->len++] = ret;
			
			// 计算 fib(n-1) ... 
			// alloc new frame for it
			struct fibFrame *xx = (struct fibFrame*)(co->stk.ptr + co->stk.len);
			co->stk.len += sizeof(*xx);
			xx->n = frame->n - 1;

			struct cont call = {
				.fn = fib,
				.label = 0,
				.frame = xx,
			};
			co->conts[co->len++] = call;
			return;
		}

	case 1:
		{
			co->stk.len -= sizeof(struct fibFrame);
			/* printf("return from result fib(n-1), n=%d, stk ptr = %d\n", frame->n, co->stk.len); */
			frame->val1 = co->res;

			// return to  + operation
			struct cont ret = {
				.fn = fib,
				.label = 2,
				.frame = frame,
			};
			co->conts[co->len++] = ret;

			// call fib(n - 2)
			struct fibFrame *yy = (struct fibFrame*)(co->stk.ptr + co->stk.len);
			co->stk.len += sizeof(*yy);
			yy->n = frame->n - 2;

			struct cont call = {
				.fn = fib,
				.label = 0,
				.frame = yy,
			};
			co->conts[co->len++] = call;
			return;
		}

	case 2:
		co->stk.len -= sizeof(struct fibFrame);
		/* printf("return from result fib(n-2), n=%d, stk ptr = %d\n", frame->n, co->stk.len); */
		co->res = co->res + frame->val1;
		return;
	}
}

int main() {
	char data[4096];
	struct stack stk = {
		.ptr = data,
		.len = 0,
		.cap = 4096,
	};

	struct cont prealloc[4096];
	struct Cora co;
	co.stk = stk;
	co.conts = prealloc;
	co.len = 0;

	struct fibFrame *frame = (struct fibFrame*)(co.stk.ptr);
	co.stk.len += sizeof(*frame);
	frame->n = 40;

	struct cont init = {
		.fn = fib,
		.label = 0,
		.frame = frame,
	};
	co.conts[co.len++] = init;

	trampoline(&co);

	printf("fib == %d\n", co.res);
	return 0;
}
