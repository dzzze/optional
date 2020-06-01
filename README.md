# Optional values and optional references

This library introduces an extension to the C++17 `std::optional` API.

`dze::optional<T, P>` has a second template parameter `Policy` which is a stateless type that defines two methods:

```cpp
static bool is_engaged(const std::byte*);
static void null_initialize(std::byte*);
```

These can be used to improvise policies that make use of the object representation of a type.

A common use case is sentinel values for trivially copyable types. Using sentinel values with the underlying type is error prone as the user needs to track usage for the semantics of optionality and manually check the sentinel value. `std::optional` may be used in such scenarios as a replacement; however, it has a more complex codegen and up to 100% memory overhead which may be particularly undesirable when the optional values have to be stored in bulk. `dze::sentinel<T, V>` overcomes these drawbacks while having essentially the same API as `std::optional`. One caveat is that this type is not `constexpr` evaluatable in C++17 as its internals use `std::memcmp` and `std::memcpy`. Check it out in action: https://godbolt.org/z/uSW-YS.

Furthermore, `dze::optional_reference<T>` fills the gap that `std::optional<T>` has left by the lack of specialization for references. `dze::optional_reference<T>` takes the approach that the standard has adopted for `std::reference_wrapper<T>` and has the underlying reference rebind on assignment.

## Acknowledgements

- The implementation of `dze::optional<T. P>` is based on the libstdc++ implementation of `std::optional<T>`.
- Test suite is based on Simon Brand's optional implementation @ https://github.com/TartanLlama/optional.
