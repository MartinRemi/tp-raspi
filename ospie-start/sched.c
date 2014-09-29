#include "sched.h"

ctx_s* current_ctx;

void init_ctx(struct ctx_s* ctx, func_t f, unsigned int stack_size) {
	(*ctx).sp = phyAlloc_alloc(stack_size);
	(*ctx).lr = phyAlloc_alloc(stack_size);
	(*ctx).f = f;
}
