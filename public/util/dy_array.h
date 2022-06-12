#pragma once

// TODO: Should we flip the order of this?
//       Having a 4 byte int and then a pointer should be 16 bytes on x64 rather than the expected 12
template<typename T>
struct dy_array
{
	unsigned int count;
	T* data;

	template<typename I>
	T& operator [] (I index)
	{
		return data[index];
	}

	template<typename I>
	const T& operator [] (I index) const
	{
		return data[index];
	}
};