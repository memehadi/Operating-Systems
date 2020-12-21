#include <stdio.h>
void g()
{
	printf("x");
}
void m()
{
	printf("F");
	g();
}
int main()
{
	m();
	return 0;
}
