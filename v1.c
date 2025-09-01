
#include <stdio.h>

struct fibFrame {
	int label;
	int val1;
	int val2;
};

int fib(struct fibFrame *frame, int n) {
 again:       
	switch (frame->label) {
	case 0:
		if (n < 2) {
			return n;
		} else {

			struct fibFrame xx = {.label = 0};
			frame->val1 = fib(&xx, n - 1);
			frame->label++;
			goto again;
		}
	case 1:
		{
			struct fibFrame yy = {.label = 0};
			frame->val2 = fib(&yy, n-2);
			frame->label++;
			goto again;
		}

	case 2:
		frame->label++;
		return frame->val1 + frame->val2;
	}
}

int main() {
	struct fibFrame frame = {.label = 0};
	int ret = fib(&frame, 41);
	printf("fib == %d\n", ret);
	return 0;
}
