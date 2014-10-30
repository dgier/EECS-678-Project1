#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

void waitFor(int secs){
	int retTime = time(0) + secs;
	while (time(0) < retTime) { ;}

}

int main()
{
	string h = "Slow World";
	waitFor(30);
	printf("%s\n", h.c_str());
	return 0;
}
