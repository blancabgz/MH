#include <math.h>
#include "random.h"

unsigned long Seed = 0L;

#define MASK 2147483647
#define PRIME 65539
#define SCALE 0.4656612875e-9

void Set_random (unsigned long x)
/* Inicializa la semilla al valor x.
   Solo debe llamarse a esta funcion una vez en todo el programa */
{
    Seed = (unsigned long) x;
}

unsigned long Get_random (void)
/* Devuelve el valor actual de la semilla */
{
    return Seed;
}

float Rand(void)
/* Genera un número aleatorio real en el intervalo [0,1[
   (incluyendo el 0 pero sin incluir el 1) */
{
    return (( Seed = ( (Seed * PRIME) & MASK) ) * SCALE );
}

int Randint(int low, int high)
/* Genera un número aleatorio entero en {low,...,high} */
{
   int aleatorio = ((int) (low + (high-(low)) * Rand()));
   while (aleatorio >= high){
      aleatorio = ((int) (low + (high-(low)) * Rand()));
   }
    return aleatorio;
}

int Randint_shuffle(int high)
/* Genera un número aleatorio entero en {low,...,high} */
{
    return (int) (high * Rand());
}

float Randfloat(float low, float high)
/* Genera un número aleatorio real en el intervalo [low,...,high[
   (incluyendo 'low' pero sin incluir 'high') */
{
   float n = (low + (high-(low))*Rand());
   if(n < 0){
      return -1.0 * n;
   }
   return n;
}
