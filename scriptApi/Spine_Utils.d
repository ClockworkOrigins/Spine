// returns the minimum of the two values a and b
func int min(var int a, var int b) {
	if (a < b) {
		return a;
	} else {
		return b;
	};
};

// returns the maximum of the two values a and b
func int max(var int a, var int b) {
	if (a > b) {
		return a;
	} else {
		return b;
	};
};

// sets value within the range of lo and hi
// if value < lo => value = lo
// if value > hi => value = hi
func int clamp(var int value, var int lo, var int hi) {
	value = max(value, lo);
	value = min(value, hi);
	return value;
};

// frees a library
func void FreeLibrary (var int ptr) {
	var int freelib;
	freelib = FindKernelDllFunction("FreeLibrary");

	if (!freelib) {
		return;
	};

	CALL_PtrParam(ptr);
	CALL__stdcall(freelib);
};
