#ifndef COLOR_H
#define COLOR_H
 
class Color
{
public:
	float red, green, blue;
 
    Color(); // default constructor
	Color(const float& red, const float& green, const float& blue);
	~Color(); // destructor
	Color& operator+=(const Color& rhs);
	Color& operator*(const float& value);
};
 
#endif