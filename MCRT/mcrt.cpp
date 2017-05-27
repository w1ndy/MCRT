#include <iostream>

#include "Scene.h"
#include "Tracer.h"

using namespace std;

int main(int argc, char *argv[])
{
	Scene s("scene01.obj");
	Tracer t(argc, argv);
	t.run(s);
	return 0;
}
