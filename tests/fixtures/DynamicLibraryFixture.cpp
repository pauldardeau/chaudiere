// Copyright Paul Dardeau, SwampBits LLC 2026
// BSD License

// A minimal shared library that exists only for TestDynamicLibrary to load
// (see the chaudiere_dynamic_library_fixture target in tests/CMakeLists.txt).
// It has no state, no static initializers and no dependencies beyond the
// platform's own runtime, and it is neither installed nor part of any public
// API.

// The one place that says how a symbol is made visible. The target builds with
// hidden default visibility, so on every platform a function is exported only
// if it is marked here.
#if defined(_WIN32)
#define CHAUDIERE_TEST_FIXTURE_EXPORT __declspec(dllexport)
#else
#define CHAUDIERE_TEST_FIXTURE_EXPORT __attribute__((visibility("default")))
#endif

// C linkage, so the symbol name is the same on every platform and compiler
// (no C++ name mangling)
extern "C" CHAUDIERE_TEST_FIXTURE_EXPORT int chaudiere_test_fixture_add(int a, int b) {
   return a + b;
}
