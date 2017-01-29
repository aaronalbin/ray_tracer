#include "Color.h"
 

Color::Color(): red(0), green(0), blue(0)
{}

Color::Color(const float& red, const float& green, const float& blue)
{
	this->red = red;
	this->green = green;
	this->blue = blue;
}

Color::~Color()
{}

Color& Color::operator+=(const Color& rhs) {
	this->red += rhs.red;
	this->green += rhs.green;
	this->blue += rhs.blue;
	return *this;
} 

Color& Color::operator*(const float& value) {
	this->red *= value;
	this->green *= value;
	this->blue *= value;
	return *this;
}
