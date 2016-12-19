#pragma once

namespace stop {

/* Call this as the first thing in a function to get the address of its
 * address
 */
auto GetCurrentFunctionAddr(int8_t magic) -> uintptr_t {
  // GCC/Clang magic
  // https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
  // Note that inlining/optimisation will defeat this.
  void *p = __builtin_extract_return_addr(__builtin_return_address(0));
  return reinterpret_cast<uintptr_t>(p) - magic;
}

namespace inthenameoflove {

auto get_offset_to_top(void *object) -> ptrdiff_t {
  // vtable pointer points to the first virtual method, but immediately
  // preceeding that are RTTI pointer (-1) and offset-to-top (-2)
  // https://mentorembedded.github.io/cxx-abi/abi.html#vtable-construction
  auto vtablep = reinterpret_cast<ptrdiff_t **>(object);
  return (*vtablep)[-2];
}

auto get_vtable_entry(void *object, ptrdiff_t index) -> uintptr_t {
  auto vtablep = reinterpret_cast<uintptr_t **>(object);
  return (*vtablep)[index];
}

template <typename> struct pmf;

template <class R, typename C, typename... Args> struct pmf<R (C::*)(Args...)> {
  using funcptr_t = R (*)(C *, Args...);

  // https://mentorembedded.github.io/cxx-abi/abi.html#member-pointers
  union {
    funcptr_t ptr;
    ptrdiff_t vtoff;
  };
  // TODO: I'm not clear on the use of this.
  ptrdiff_t adj;

  auto is_virtual() const -> bool {
    return (vtoff % 2) == 1;
  };
  auto vtoffset() const -> ptrdiff_t {
    return vtoff - 1;
  };

  auto devirtualise(C *obj) const -> funcptr_t {
    return reinterpret_cast<funcptr_t>(get_vtable_entry(obj, vtoffset()));
  }

  auto shiftthis(C *obj) const -> C *{
    return reinterpret_cast<C *>(reinterpret_cast<uintptr_t>(obj) +
                                 get_offset_to_top(obj));
  }

  // TODO: nullptr is "I have no idea how to get this..."
  auto getptr() const -> funcptr_t { return is_virtual() ? nullptr : ptr; }

  auto getptr(C *obj) const -> funcptr_t {
    return is_virtual() ? devirtualise(obj) : ptr;
  }

  auto getthis(C *obj) const -> C *{
    return is_virtual() ? shiftthis(obj) : obj;
  }
};
};

// Itanium ABI
template <typename... Args> using pmf = inthenameoflove::pmf<Args...>;

// Return a non-member function taking a C* and Args, equivalent to calling C’s
// member function with Args
template <class C, typename R, typename... Args>
auto GetFunctionPointer(R (C::*fp)(Args...)) -> R (*)(C *, Args...) {
  using pmf_t = pmf<decltype(fp)>;

  auto *pPMF = reinterpret_cast<pmf_t *>(&fp);
  return pPMF->getptr();
}

// Return a non-member function taking a C* and Args, equivalent to calling C’s
// member function with Args
template <class C, class D, typename R, typename... Args>
auto GetFunctionPointer(D *obj, R (C::*fp)(Args...)) -> R (*)(C *, Args...) {
  using pmf_t = pmf<decltype(fp)>;

  auto *pPMF = reinterpret_cast<pmf_t *>(&fp);
  return pPMF->getptr(static_cast<C *>(obj));
}

/*
// Return the C* that would be used if C’s member pointer was called on a D*.
template <class C, class D, typename R, typename... Args>
auto GetAdjustedThisPointer(D *obj, R (C::*fp)(Args...)) -> C *{
  using pmf_t = pmf<decltype(fp)>;

  auto *pPMF = reinterpret_cast<pmf_t *>(&fp);
  return pPMF->getthis(obj);
}
*/
// Return the C* that would be used if C’s member pointer was called on a D*.
template <class C, class D, typename R, typename... Args>
auto GetAdjustedThisPointer(D *obj, R (C::*fp)(Args...)) -> C *{
  // TODO: Only allow this for upcasts, rather than relying on SFINAE.
  // This seems simple... Probably complex in the virtual-base case,
  // more like the above version.
  return obj;
}
};
