/*
INITIAL CODE
int
divide(int dividend,int divisor)
{
int result = 0;
int remainder = dividend;

while (remainder >= divisor) {
result++;
remainder -= divisor;
}
return result;
}
int compute_volume( int rad)
{
int rad3 = rad * rad * rad;
return divide(4*355*rad3, 3*113);
}
int kmain(void)
{
int radius = 5;
int volume;
volume = compute_volume(radius);
return volume;
}*/

#include "sched.h"

struct ctx_s ctx_ping;
struct ctx_s ctx_pong;
struct ctx_s ctx_init;

void ping()
{
	int cpt = 0;

	while( 1 ) {
		cpt++;
		switch_to(&ctx_pong);
	}
}

void pong()
{
	int cpt = 1;

	while ( 1 ) {
		cpt+=2;
		switch_to(&ctx_ping);
	}
}
// ------------------------------------------------------------------------
int kmain (void)
{
	init_hw();
	init_ctx(&ctx_ping, ping, STACK_SIZE);
	init_ctx(&ctx_pong, pong, STACK_SIZE);
	current_ctx = &ctx_init;
	switch_to(&ctx_ping);
	/*
	Pas
	atteignable
	vues
	nos
	2
	fonctions
	*/
	return (0);
}

