#include "sched.h"
#include "hw.h"

struct pcb_s* first = NULL;
struct pcb_s* last = NULL;
struct pcb_s* current_process = NULL;

void idle()
{
	while(1);
}

void init_ctx(struct ctx_s* ctx, func_t f, unsigned int stack_size) {
	ctx->sp = phyAlloc_alloc(stack_size) + stack_size - 14 * 4;
	ctx->link_register = f;
	ctx->f = f;
}

void __attribute__ ((naked)) switch_to(struct ctx_s* ctx) {
	// Sauvegarde des registres
	__asm("push {r0-r12}");	

	// Sauvegarde du contexte courant
	__asm("mov %0, sp" : "=r"(current_ctx->sp));
	__asm("mov %0, lr" : "=r"(current_ctx->link_register));

	// Changer contexte courant
	current_ctx = ctx;

	// Restauration du contexte
	__asm("mov sp, %0" : : "r"(ctx->sp));
	__asm("mov lr, %0" : : "r"(ctx->link_register));

	// Restauration des registres
	__asm("pop {r0-r12}");	

	// Sauter à l'adresse de retour
	__asm("bx lr");
}

void init_pcb(struct pcb_s* pcb, func_t f, unsigned int stack_size, void* args) {	
	pcb->state = NEW;	
	pcb->ctx = phyAlloc_alloc(sizeof(struct ctx_s));
	init_ctx(pcb->ctx, f, stack_size);
	pcb->size = stack_size;
	pcb->args = args;
}

void create_process(func_t f, void* args, unsigned int stack_size) {
	DISABLE_IRQ();
	struct pcb_s* pcb = phyAlloc_alloc(sizeof(struct pcb_s));
	if(first == NULL) {
		first = pcb;
		last = pcb;
	} else {
		last->next = pcb;
	}
	pcb->next = first;
	init_pcb(pcb, f, stack_size, args);
	set_tick_and_enable_timer();	
	ENABLE_IRQ();
}

void start_current_process() {
	current_process->state = RUNNING;
	current_process->ctx->f(current_process->args);
	end_current_process();
}

void end_current_process() {
	/*if(current_process->next == current_process) {
		first = NULL;
	} else {
		uint32* newNext = current_process->next;
		struct pcb_s* a = current_process;
		while(a->next != current_process) {
			a = a->next;
		}
		a->next = newNext;
	}*/
	current_process->state = TERMINATED;
	int allTerminated = 1;
	struct pcb_s* pcb = first;
	do {
		allTerminated = (pcb->state != TERMINATED) ? 0 : 1;
		pcb = pcb->next;
	} while(pcb != first && allTerminated);
	if(allTerminated) create_process(idle, NULL, STACK_SIZE);
	ctx_switch();
}

void elect() {
	struct pcb_s* pcb = first;
	if(pcb != NULL) {
	// TODO: free all memory and handle pcb list (terminated process)
		/*do {
			if(pcb != current_process) {
				phyAlloc_free(pcb->ctx, pcb->size);
			}
			pcb = pcb->next;
		} while(pcb != first);*/
	}

	if(current_process != NULL) {
		current_process = current_process->next;
	} else {
		current_process = first;
	}
	while(current_process->state == TERMINATED) {
		current_process = current_process->next;
	}
}

void start_sched() {
	ENABLE_IRQ();
	set_tick_and_enable_timer();
}

void __attribute__ ((naked)) ctx_switch() {
	
	// Sauvegarde des registres
	__asm("push {r0-r12}");	


	// Sauvegarde du contexte du current_process
	if(current_process != NULL) {
		__asm("mov %0, sp" : "=r"(current_process->ctx->sp));
		__asm("mov %0, lr" : "=r"(current_process->ctx->link_register));
		current_process->state = WAITING;
	}

	// Demande au scheduler d'élire un nouveau processus
	elect();

	// Restaure le contexte du processus elu
	if(current_process != NULL) {
		__asm("mov sp, %0" : : "r"(current_process->ctx->sp));
		__asm("mov lr, %0" : : "r"(current_process->ctx->link_register));

		if(current_process->state == READY) {
			start_current_process();
		} else {
			// Sauter à l'adresse de retour
			current_process->state = RUNNING;

			// Restauration des registres
			__asm("pop {r0-r12}");

			__asm("bx lr");
		}
	}	
}

void ctx_switch_from_irq() {
	DISABLE_IRQ();

	__asm("sub lr, lr, #4");
	__asm("srsdb sp!, #0x13");
	__asm("cps #0x13");
		
	// Sauvegarde des registres
	__asm("push {r0-r12}");	


	// Sauvegarde du contexte du current_process
	if(current_process != NULL) {
		__asm("mov %0, sp" : "=r"(current_process->ctx->sp));
		__asm("mov %0, lr" : "=r"(current_process->ctx->link_register));
		current_process->state = WAITING;
	}

	// Demande au scheduler d'élire un nouveau processus
	elect();

	// Restaure le contexte du processus elu
	if(current_process != NULL) {
		__asm("mov sp, %0" : : "r"(current_process->ctx->sp));
		__asm("mov lr, %0" : : "r"(current_process->ctx->link_register));
	}

	// Restauration des registres
	__asm("pop {r0-r12}");

	set_tick_and_enable_timer();	
	ENABLE_IRQ();

	if(current_process->state == NEW) {
		start_current_process();
	}
	current_process->state = RUNNING;
	__asm("rfeia sp!");

	
}
