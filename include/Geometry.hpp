#ifndef __GEOMETRY_HPP_
#define __GEOMETRY_HPP_

// Geometry Header
// Header only library containing definition of Point and it's alias Size.
// 	as well as Rectangle.


#include <Arduino.h>


namespace Utility {

// A utility to tell if an integer type is signed or unsigned at runtime or compile time.
template <typename T>
struct is_signed {
	static constexpr bool value { (T(-1) < T(0)) }; 
};

template <typename T>
struct is_unsigned {
	static constexpr bool value { (T(-1) > T(0)) };
};

}


// A point
// Templated to allow for changing the stored data type easily throughout the whole
//	program.
template <typename T>
struct Point
#if (DEBUG == YES) 
: public Printable	// Inheriting from printable can allow you to print() a point.
#endif 
{
	constexpr Point() : y{0}, x{0} {}
	constexpr Point(T y, T x) : y{y}, x{x} {}

	template <typename OtherPointDataType>
	constexpr Point(const Point<OtherPointDataType>& other) : y{other.y}, x{other.x}  {}

	// The Coordinates
	T y, x; 

	// Override equality operator to compare points.
	constexpr bool operator==(const Point& other) const { return (other.y == y && other.x == x); }
	constexpr bool operator!=(const Point& other) const { return !(*this == other); }
	// Override addition assignment operator to add and assign.
	constexpr void operator+=(const Point& other) { y += other.y; x += other.x; }
	constexpr void operator-=(const Point& other) { y -= other.y; x -= other.x; }

#if (DEBUG == YES)
size_t printTo(Print& p) const {
	size_t cnt { 0 };
	cnt += p.print('(');
	cnt += p.print(y);
	cnt += p.print(", ");
	cnt += p.print(x);
	cnt += p.print(')');
	return cnt;
}
#endif
};


template <typename T> using Size = Point<T>;

template <typename PointT, typename DataT = decltype(PointT::x)>
struct Rectangle 
#if (DEBUG == YES)
: public Printable
#endif
{
	using SizeT = Size<DataT>;
private:
// Origin is tl
	PointT mOrigin;
	SizeT mSize;
public:
	constexpr Rectangle() : mOrigin{PointT{0,0}}, mSize{PointT{0,0}} {}
	constexpr Rectangle(DataT y, DataT x, DataT height, DataT width) : mOrigin{y, x}, mSize{height, width} {}
	constexpr Rectangle(const PointT& tl, const PointT& tr, const PointT& bl, const PointT& br) : mOrigin{tl.y, tl.x}, mSize{ bl.y - tl.y, tr.x - tl.x } {} 
	constexpr Rectangle(const PointT& origin, const SizeT& size) : mOrigin{origin}, mSize{size} {}
	constexpr Rectangle(const SizeT& size) : mOrigin{0, 0}, mSize{ size } {}

	template <typename OtherPointType>
	constexpr Rectangle(const Rectangle<OtherPointType>& other) {
		this->mOrigin = other.origin();
		this->mSize = other.size();
	}

	constexpr PointT tl() const { return mOrigin; }
	constexpr PointT bl() const { return { mOrigin.y + mSize.y, mOrigin.x }; }
	constexpr PointT tr() const { return { mOrigin.y, mOrigin.x + mSize.x }; }
	constexpr PointT br() const { return { static_cast<DataT>(mOrigin.y + mSize.y), static_cast<DataT>(mOrigin.x + mSize.x) }; }
	constexpr PointT origin() const { return mOrigin; }
	constexpr PointT centre() const { 
		return { 	static_cast<DataT>(mOrigin.y + (mSize.y / 2)),
		 			static_cast<DataT>(mOrigin.x + (mSize.x / 2)) 
		};
	}

	constexpr SizeT size() const { 	return mSize; }
	constexpr DataT height() const { return mSize.y; }
	constexpr DataT width() const { return mSize.x; }
	constexpr DataT minY() const { return mOrigin.x; }
	constexpr DataT minX() const { return mOrigin.y; }
	constexpr DataT maxY() const { return br().y; }
	constexpr DataT maxX() const { return br().x; }

	constexpr Rectangle& grow(DataT amountY, DataT amountX) { 
			mOrigin.y -= amountY;
			mOrigin.x -= amountX;
			mSize.y += (amountY + amountY);
			mSize.x += (amountX + amountX);
			return *this;
	}

	constexpr Rectangle& grow(DataT amount) { return grow(amount, amount); }
	

	template <typename OtherPointDataType>
	constexpr Rectangle& centreOn(const Point<OtherPointDataType>& point) { 
			
		mOrigin.x = point.x - (mSize.x / 2);
		mOrigin.y = point.y - (mSize.y / 2);
		return *this;
	}
	
	template <typename OtherPointType>
	constexpr Rectangle& centreOn(const Rectangle<OtherPointType>& other) { 
		return centreOn(other.centre()); 
	}


#if (DEBUG == YES)
	size_t printTo(Print& p) const {
		size_t r {0};
		r += p.print("origin: ");
		r += p.print(mOrigin);
		r += p.print(", size: ");
		r += p.print(mSize);
		return r;
	}
#endif

};


#endif // __GEOMETRY_HPP_
