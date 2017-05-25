#pragma once

#include <array>
#include <initializer_list>
#include <cassert>

template<size_t N>
class Vector
{
protected:
	std::array<float, N> _data;

public:
	Vector() {};

	Vector(std::initializer_list<float> const &values)
	{
		size_t index = 0;
		for (auto const &p : values) {
			assert(index < N && "initializer list out of bound");
			_data[index++] = p;
		}
	}

	Vector(Vector const &v) : _data(v._data) {};
	Vector(Vector const &&v) : _data(std::move(v._data)) {};

	constexpr size_t size()
	{
		return N;
	}

	float &operator[](size_t i)
	{
		assert(i < N && "vector.get: argument out of bound");
		return _data[i];
	}

	float operator[](size_t i) const
	{
		assert(i < N && "vector.get: argument out of bound");
		return _data[i];
	}
};

class Vertex3f : public Vector<3>
{
private:
	using Vector<3>::_data;

public:
	float &x;
	float &y;
	float &z;

public:
	Vertex3f() : x(_data[0]), y(_data[1]), z(_data[2]) {};

	Vertex3f(float x, float y, float z)
		: x(_data[0]), y(_data[1]), z(_data[2])
	{
		_data[0] = x, _data[1] = y, _data[2] = z;
	}
};

class TextureCoord : public Vector<2>
{
private:
	using Vector<2>::_data;

public:
	float &u;
	float &v;

public:
	TextureCoord() : u(_data[0]), v(_data[0]) {};

	TextureCoord(float u, float v)
		: u(_data[0]), v(_data[0])
	{
		_data[0] = u, _data[1] = v;
	}
};