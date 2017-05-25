#pragma once

class Tracer
{
public:
	Tracer(int &argc, char *argv[]);

	void run();

private:
	void onUpdating();
	void onResized(int width, int height);
	void onIdle();


private:
	int _width;
	int _height;

private:
	static Tracer *_instance;
	static void _displayFn();
	static void _resizeFn(int width, int height);
	static void _idleFn();
};
