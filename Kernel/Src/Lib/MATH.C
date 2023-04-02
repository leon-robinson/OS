#include <Lib/MATH.H>

u16 ClampU16(u16 val, u16 min, u16 max) {
	return val < min ? min : val > max ? max : val;
}

int Abs(int x) {
	return x < 0 ? -x : x;
}
