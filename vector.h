#include<iostream>
#include <cmath>

class Vector;

class Point
{
public:
	double x, y, z;

	Point() : x(0), y(0), z(0) {}

	inline Point(double a, double b, double c) : x(a), y(b), z(c) {}
	inline Point(double a, double b) : x(a), y(b), z(0) {}

	Point operator*(const double& d) const;
	Point operator*=(const double& d) const;
	Vector operator-(const Point& p) const;
	Point operator+(const Vector& v) const;
	Point operator-(const Vector& v) const;
	Point operator+(const Point& p) const;
};

class Vector
{
public:

	double x, y, z;

	Vector() : x(0), y(0), z(0) {}

	inline Vector(double a, double b, double c) : x(a), y(b), z(c) {}
	inline Vector(double a, double b): x(a), y(b), z(0) {}

	Vector operator*(const Vector& v) const
	{
		Vector r;

		r.x = this->y * v.z - this->z * v.y;
		r.y = this->z * v.x - this->x * v.z;
		r.z = this->x * v.y - this->y * v.x;

		return r;
	}

	double operator%(const Vector& v) const
	{
		return this->x * v.x + this->y * v.y + this->z * v.z;
	}

	Vector operator-() const
	{
		Vector r = *this;

		r *= -1.0;

		return r;
	}

	Vector operator-(const Vector& v) const
	{
		Vector r;

		r.x = this->x - v.x;
		r.y = this->y - v.y;
		r.z = this->z - v.z;

		return r;
	}

	Vector operator-=(const Vector& v) const
	{
		Vector r;

		r.x = this->x - v.x;
		r.y = this->y - v.y;
		r.z = this->z - v.z;

		return r;
	}

	Point operator-(const Point& p) const
	{
		Point r = p;

		r.x -= x;
		r.y -= y;
		r.z -= z;

		return r;
	}

	Vector operator+(const Vector& v) const
	{
		Vector r;

		r.x = this->x + v.x;
		r.y = this->y + v.y;
		r.z = this->z + v.z;

		return r;
	}

	Vector operator+=(const Vector& v) const
	{
		Vector r;

		r.x = this->x + v.x;
		r.y = this->y + v.y;
		r.z = this->z + v.z;

		return r;
	}

	Point operator+(const Point& p) const
	{
		Point r = p;

		r.x += x;
		r.y += y;
		r.z += z;

		return r;
	}

	Vector operator*(const double& d) const
	{
		Vector r;

		r.x = this->x * d;
		r.y = this->y * d;
		r.z = this->z * d;

		return r;
	}

	Vector operator*=(const double& d) const
	{
		Vector r;

		r.x = this->x * d;
		r.y = this->y * d;
		r.z = this->z * d;

		return r;
	}

	Vector operator/(const double& d) const
	{
		Vector r;

		r.x = this->x / d;
		r.y = this->y / d;
		r.z = this->z / d;

		return r;
	}

	Vector operator/=(const double& d) const
	{
		Vector r;

		r.x = this->x / d;
		r.y = this->y / d;
		r.z = this->z / d;

		return r;
	}

	inline double norm() const
	{
		double n = this->x * this->x + this->y * this->y + this->z * this->z;
		return sqrt(n);
	}
};


std::ostream& operator<<(std::ostream& os, const Vector&v)
{
	os<<"["<<v.x<<", "<<v.y<<", "<<v.z<<"]";

	return os;
}

Vector operator*(double d, const Vector& v)
{
	Vector r = v;
	return r *= d;
}

Vector operator/(double d, const Vector& v)
{
	Vector r = v;
	return r /= d;
}

Vector normalize(const Vector& v)
{
	Vector unit;

	double n = v.norm();

	if( n == 0 )
		return unit;

	unit = v / n;

	return unit;
}

Vector perp(const Vector& v)
{
	Vector perp;

	perp.x = -v.y;
	perp.y = v.x;
	perp.z = v.z;

	return perp;
}

std::ostream& operator<<(std::ostream& os, const Point& p)
{
	os<<"("<<p.x<<", "<<p.y<<", "<<p.z<<")";

	return os;
}

Point operator*(double d, const Point& p)
{
	Point r = p;
	return r *= d;
}

Point Point::operator*(const double& d) const
{
	Point r = *this;

	r.x *= d;
	r.y *= d;
	r.z *= d;

	return r;
}

Point Point::operator*=(const double& d) const
{
	Point r = *this;

	r = r * d;

	return r;
}

Vector Point::operator-(const Point& p) const
{
	Vector r;

	r.x = x - p.x;
	r.y = y - p.y;
	r.z = z - p.z;

	return r;
}

Point Point::operator+(const Vector& v) const
{
	Point r = v + *this;

	return r;
}

Point Point::operator-(const Vector& v) const
{
	Point r = v - *this;

	return r;
}

Point Point::operator+(const Point& p) const
{
	Point r;

	r.x = this->x + p.x;
	r.y = this->y + p.y;
	r.z = this->z + p.z;

	return r;
}