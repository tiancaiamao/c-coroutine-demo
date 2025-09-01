#include <stdio.h>

struct fibFrame {
	int label;
	int val1;
	int val2;
};

struct stack {
	char *ptr;
	int len;
	int cap;
};


int fib(struct stack *stk, struct fibFrame *frame, int n) {
 again:       
	switch (frame->label) {
	case 0:
		if (n < 2) {
			return n;
		} else {			
			// call fib(n-1) ...
			struct fibFrame *xx = (struct fibFrame*)(stk->ptr + stk->len);
			stk->len += sizeof(*xx);
			xx->label = 0;
			frame->val1 = fib(stk, xx, n - 1);
			stk->len -= sizeof(*xx);

			// return to  fib(n - 2)
			frame->label++;
			goto again;
		}
	case 1:
		// return the the next + operation
		frame->label++;

		// call fib(n - 2)
		struct fibFrame *yy = (struct fibFrame*)(stk->ptr + stk->len);
		stk->len += sizeof(*yy);
		yy->label = 0;

		frame->val2 = fib(stk, yy, n - 2);
		stk->len -= sizeof(*yy);
		goto again;

	case 2:
		return frame->val1 + frame->val2;
	}
}

int main() {
	char data[4096];
	struct stack stk = {
		.ptr = data,
		.len = 0,
		.cap = 4096,
	};

	struct fibFrame *frame = (struct fibFrame*)(stk.ptr + stk.len);
	stk.len += sizeof(*frame);
	frame->label = 0;


	int ret = fib(&stk, frame, 41);

	printf("fib == %d\n", ret);
	return 0;
}
