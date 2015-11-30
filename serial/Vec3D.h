
struct Vec3D{
	
	float x;
	float y;
	float z;

	Vec3D(const float& _x, const float& _y, const float& _z) : x(_x), y(_y), z(_z)
};

// plus
inline Vec3D operator+(const Vec3D& a, const Vec3D& b){
	return Vec3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

// substract
inline Vec3D operator-(const Vec3D& a, const Vec3D& b){
	return Vec3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

// inversion
inline Vec3D operator-(const Vec3D& a, const Vec3D& b){
	return Vec3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

// assignment-add
inline Vec3D operator+=(const Vec3D& a, const Vec3D& b){
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

// assignment-substraction
inline Vec3D operator-=(const Vec3D& a, const Vec3D& b){
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// equality test
inline bool operator==(const Vec3D& a, const Vec3D& b){
    return (  (a.x == b.x) && (a.y == b.y) && (a.z == b.z)  ) ;
}

// inequality test
inline bool operator!=(const Vec3D& a, const Vec3D& b){
    return (  (a.x != b.x) || (a.y != b.y) || (a.z != b.z)  ) ;
}




/*
	Vector Math Section
*/


// dot product
inline float dot(const Vec3D& a, const Vec3D& b){
    return (  a.x * b.x + a.y * b.y + a.z * b.z  ) ;
}

// cross product
inline Vec3D cross(const Vec3D& a, const Vec3D& b){
    return Vec3D (	a.y * b.z - a.z * b.y,
                  	a.z * b.x - a.x * b.z,
                  	a.x * b.y - a.y * b.x);
}













