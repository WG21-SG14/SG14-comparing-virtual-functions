// g++ gfp.cpp -std=c++11 -Wno-pmf-conversions -Wall -o gfp && ./gfp
#define CATCH_CONFIG_MAIN
// https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp
#include "catch.hpp"

// All the magic and horror is here
#include "gfp.hpp"

uintptr_t f(int, char, float) {
// MAGIC
#if (__GNUC__ && __cplusplus) && !__clang__
  // GCC and Clang disagree about this magic. This is GCC's value
  return stop::GetCurrentFunctionAddr(31);
#else
  // GCC and Clang disagree about this magic. This is Clang's value
  return stop::GetCurrentFunctionAddr(34);
#endif
};

SCENARIO("Test-supporting things work correctly with non-member functions",
         "[support]") { GIVEN("a free function that returns its address") {
  WHEN("the function is called") { auto a = f(0, 'a', 1.f);

THEN("the returned value is the same as the address of the function") {
  REQUIRE(a == reinterpret_cast<uintptr_t>(&f));
}
}
}
}
;

SCENARIO(
    "Pointers to non-virtual functions can be extracted from member pointers",
    "[pmf-nv-extract]") { GIVEN("a class with a member function") {
  class A{ public : uintptr_t f(int a, char b, float c) {
#if (__GNUC__ && __cplusplus) && !__clang__
    // GCC and Clang disagree about this magic. This is GCC's value
    auto result = stop::GetCurrentFunctionAddr(35);
#else
    // GCC and Clang disagree about this magic. This is Clang's value
    auto result = stop::GetCurrentFunctionAddr(45);
#endif
a_ = a;
b_ = b;
c_ = c;
return result;
}

int a_;
char b_;
float c_;
}
;

WHEN("the member function's address is extracted without an object") {
  auto extractedFP = stop::GetFunctionPointer(&A::f);
  auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
  THEN("it matches the gcc extension's result") {
    using fp_t = uintptr_t (*)(A *, int, char, float);

    auto memp = &A::f;
    auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(&A::f));

    REQUIRE(extractedFPval == gccFP);
  }
#endif
  THEN("it matches the self-determined address of the function") {
    auto a = A{};
    auto realFP = a.f(0, 'a', 1.f);
    REQUIRE(extractedFPval == realFP);
  }
}

WHEN("the member function's address is extracted with an object") {
  auto a = A{};
  auto extractedFP = stop::GetFunctionPointer(&a, &A::f);
  auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
  THEN("it matches the gcc extension's result") {
    using fp_t = uintptr_t (*)(A *, int, char, float);

    auto memp = &A::f;
    auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(a.*memp));

    REQUIRE(extractedFPval == gccFP);
  }
#endif
  THEN("it matches the self-determined address of the function") {
    auto realFP = a.f(0, 'a', 1.f);
    REQUIRE(extractedFPval == realFP);
  }
  AND_WHEN("the object pointer is adjusted for the call") {
    auto adjusted_ap = stop::GetAdjustedThisPointer(&a, &A::f);
    THEN("it matches the address of the original object") {
      REQUIRE(adjusted_ap == reinterpret_cast<A *>(&a));
    }

    AND_WHEN(
        "the extracted function is called via the adjusted object pointer") {
      a.f(0, 'a', 1.f);
      REQUIRE(a.a_ == 0);
      REQUIRE(a.b_ == 'a');
      REQUIRE(a.c_ == 1.f);

      extractedFP(adjusted_ap, 5, 'w', 72.f);

      THEN("the correct effects are observed") {
        REQUIRE(a.a_ == 5);
        REQUIRE(a.b_ == 'w');
        REQUIRE(a.c_ == 72.f);
      }
    }
  }
}
}
}
;

SCENARIO("Pointers to virtual functions can be extracted from member pointers",
         "[pmf-virt-extract]") {
  GIVEN("a class with a virtual member function") {
    class A{ public : virtual uintptr_t f(int a, char b, float c) {
#if (__GNUC__ && __cplusplus) && !__clang__
      // GCC and Clang disagree about this magic. This is GCC's value
      auto result = stop::GetCurrentFunctionAddr(35);
#else
      // GCC and Clang disagree about this magic. This is Clang's value
      auto result = stop::GetCurrentFunctionAddr(45);
#endif
a_ = a;
b_ = b;
c_ = c;
return result;
}

int a_;
char b_;
float c_;
}
;

#if 0
// I don't think this is possible... The *compiler* knows, but it won't tell
// me without GCC's extension, and I don't know how to say "Give me the vtable
// you would construct for C", without constructing C.
WHEN("the member function's address is extracted without an object") {
  auto extractedFP = stop::GetFunctionPointer(&A::f);
  auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
  THEN("it matches the gcc extension's result") {
    using fp_t = uintptr_t (*)(A *, int, char, float);

    auto memp = &A::f;
    auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(&A::f));

    REQUIRE(extractedFPval == gccFP);
  }
#endif
  THEN("it matches the self-determined address of the function") {
    auto a = A{};
    auto realFP = a.f(0, 'a', 1.f);
    REQUIRE(extractedFPval == realFP);
  }
}
#endif

WHEN("the member function's address is extracted with an object") {
  auto a = A{};
  auto extractedFP = stop::GetFunctionPointer(&a, &A::f);
  auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
  THEN("it matches the gcc extension's result") {
    using fp_t = uintptr_t (*)(A *, int, char, float);

    auto memp = &A::f;
    auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(a.*memp));

    REQUIRE(extractedFPval == gccFP);
  }
#endif
  THEN("it matches the self-determined address of the function") {
    auto realFP = a.f(0, 'a', 1.f);
    REQUIRE(extractedFPval == realFP);
  }

  AND_WHEN("the object pointer is adjusted for the call") {
    auto adjusted_ap = stop::GetAdjustedThisPointer(&a, &A::f);
    THEN("it matches the address of the original object") {
      REQUIRE(adjusted_ap == reinterpret_cast<A *>(&a));
    }

    AND_WHEN(
        "the extracted function is called via the adjusted object pointer") {
      a.f(0, 'a', 1.f);
      REQUIRE(a.a_ == 0);
      REQUIRE(a.b_ == 'a');
      REQUIRE(a.c_ == 1.f);

      extractedFP(adjusted_ap, 5, 'w', 72.f);

      THEN("the correct effects are observed") {
        REQUIRE(a.a_ == 5);
        REQUIRE(a.b_ == 'w');
        REQUIRE(a.c_ == 72.f);
      }
    }
  }
}
GIVEN("a simple subclass that does not override the member function") {
  class B : public A {};
  WHEN("the member function's address is extracted with an object") {
    auto b = B{};
    auto extractedFP = stop::GetFunctionPointer(&b, &A::f);
    auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
    THEN("it matches the gcc extension's result") {
      using fp_t = uintptr_t (*)(A *, int, char, float);

      auto memp = &A::f;
      auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(b.*memp));

      REQUIRE(extractedFPval == gccFP);
    }
#endif
    THEN("it matches the self-determined address of the function") {
      auto realFP = b.f(0, 'a', 1.f);
      REQUIRE(extractedFPval == realFP);
    }

    AND_WHEN("the object pointer is adjusted for the call") {
      auto adjusted_bp = stop::GetAdjustedThisPointer(&b, &A::f);
      THEN("it matches the address of the original object") {
        REQUIRE(adjusted_bp == reinterpret_cast<A *>(&b));
      }

      AND_WHEN(
          "the extracted function is called via the adjusted object pointer") {
        b.f(0, 'a', 1.f);
        REQUIRE(b.a_ == 0);
        REQUIRE(b.b_ == 'a');
        REQUIRE(b.c_ == 1.f);

        extractedFP(adjusted_bp, 5, 'w', 72.f);

        THEN("the correct effects are observed") {
          REQUIRE(b.a_ == 5);
          REQUIRE(b.b_ == 'w');
          REQUIRE(b.c_ == 72.f);
        }
      }
    }
  }
}
GIVEN("a simple subclass that overrides the member function") {
  class B : public A {
  public:
    B() : A(), d_(1) {}
    uintptr_t f(int a, char b, float c) override {
#if (__GNUC__ && __cplusplus) && !__clang__
      // GCC and Clang disagree about this magic. This is GCC's value
      auto result = stop::GetCurrentFunctionAddr(35);
#else
      // GCC and Clang disagree about this magic. This is Clang's value
      auto result = stop::GetCurrentFunctionAddr(45);
#endif
      a_ = a + 1;
      b_ = b + 1;
      c_ = c * 2.f;
      d_ *= 2;
      return result;
    }

    int d_;
  };
  WHEN("the member function's address is extracted with an object") {
    auto b = B{};
    auto extractedFP = stop::GetFunctionPointer(&b, &A::f);
    auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
    THEN("it matches the gcc extension's result") {
      using fp_t = uintptr_t (*)(A *, int, char, float);

      auto memp = &A::f;
      auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(b.*memp));

      REQUIRE(extractedFPval == gccFP);
    }
#endif
    THEN("it matches the self-determined address of the function") {
      auto realFP = b.f(0, 'a', 1.f);
      REQUIRE(extractedFPval == realFP);
    }

    AND_WHEN("the object pointer is adjusted for the call") {
      auto adjusted_bp = stop::GetAdjustedThisPointer(&b, &A::f);
      THEN("it matches the address of the original object") {
        REQUIRE(adjusted_bp == reinterpret_cast<A *>(&b));
      }

      AND_WHEN(
          "the extracted function is called via the adjusted object pointer") {
        b.f(0, 'a', 1.f);
        b.d_ = 1;
        REQUIRE(b.a_ == 1);
        REQUIRE(b.b_ == 'b');
        REQUIRE(b.c_ == 2.f);
        REQUIRE(b.d_ == 1);

        extractedFP(adjusted_bp, 5, 'w', 72.f);

        THEN("the correct effects are observed") {
          REQUIRE(b.a_ == 6);
          REQUIRE(b.b_ == 'x');
          REQUIRE(b.c_ == 144.f);
          REQUIRE(b.d_ == 2);
        }
      }
    }
  }
}
GIVEN("a complex subclass that does not override the member function") {
  class Intrusive {
  public:
    virtual void set(int x) { bananas_ = x; }
    int bananas_;
  };
  class B : public Intrusive, public A {};
  WHEN("the member function's address is extracted with an object") {
    auto b = B{};
    auto extractedFP = stop::GetFunctionPointer(&b, &A::f);
    auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
    THEN("it matches the gcc extension's result") {
      using fp_t = uintptr_t (*)(A *, int, char, float);

      auto memp = &A::f;
      auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(b.*memp));

      REQUIRE(extractedFPval == gccFP);
    }
#endif
    THEN("it matches the self-determined address of the function") {
      auto realFP = b.f(0, 'a', 1.f);
      REQUIRE(extractedFPval == realFP);
    }

    AND_WHEN("the object pointer is adjusted for the call") {
      auto adjusted_bp = stop::GetAdjustedThisPointer(&b, &A::f);
      THEN("it does not match the address of the original object") {
        REQUIRE(adjusted_bp != reinterpret_cast<A *>(&b));
      }

      AND_WHEN(
          "the extracted function is called via the adjusted object pointer") {
        b.f(0, 'a', 1.f);
        REQUIRE(b.a_ == 0);
        REQUIRE(b.b_ == 'a');
        REQUIRE(b.c_ == 1.f);

        extractedFP(adjusted_bp, 5, 'w', 72.f);

        THEN("the correct effects are observed") {
          REQUIRE(b.a_ == 5);
          REQUIRE(b.b_ == 'w');
          REQUIRE(b.c_ == 72.f);
        }
      }
    }
  }
}
GIVEN("a complex subclass that overrides the member function") {
  class Intrusive {
  public:
    virtual void set(int x) { bananas_ = x; }
    int bananas_;
  };
  class B : public Intrusive, public A {
  public:
    B() : A(), d_(1) {}
    uintptr_t f(int a, char b, float c) override {
#if (__GNUC__ && __cplusplus) && !__clang__
      // GCC and Clang disagree about this magic. This is GCC's value
      auto result = stop::GetCurrentFunctionAddr(35);
#else
      // GCC and Clang disagree about this magic. This is Clang's value
      auto result = stop::GetCurrentFunctionAddr(45);
#endif
      a_ = a + 1;
      b_ = b + 1;
      c_ = c * 2.f;
      d_ *= 2;
      return result;
    }

    int d_;
  };
  WHEN("the member function's address is extracted with an object") {
    auto b = B{};
    auto extractedFP = stop::GetFunctionPointer(&b, &A::f);
    // XXX: This ends up pointing to a thunk which subtracts 0x10 from 'this'
    // and jumps to the real address of B::f.
    // cf. http://mentorembedded.github.io/cxx-abi/abi-examples.html#vcall
    // "prior to calling any vtable entry, the this pointer has been adjusted
    // to point to the subobject corresponding to the vtable from which the
    // vptr is fetched."
    // So the thunk "thunk(A*, B::f)" gets an A*, turns it back into a B*,
    // and jumps to B::f.
    // Note that the -0x10 is _also_ available in the vtable, see
    // stop::inthenameoflove::get_offset_to_top
    // But I can see no way to get the real address of B::f from
    // run-time-available data without parsing the thunk.
    auto extractedFPval = reinterpret_cast<uintptr_t>(extractedFP);
#if (__GNUC__ && __cplusplus) && !__clang__
    THEN("it matches the gcc extension's result") {
      // XXX: Passes, so it's also pointing to a thunk
      using fp_t = uintptr_t (*)(A *, int, char, float);

      auto memp = &A::f;
      auto gccFP = reinterpret_cast<uintptr_t>(reinterpret_cast<fp_t>(b.*memp));

      REQUIRE(extractedFPval == gccFP);
    }
#endif
    THEN("it matches the self-determined address of the function") {
      // XXX: Fails, because realFP is post-thunk
      auto realFP = b.f(0, 'a', 1.f);
      REQUIRE(extractedFPval == realFP);
    }

    AND_WHEN("the object pointer is adjusted for the call") {
      auto adjusted_bp = stop::GetAdjustedThisPointer(&b, &A::f);
      THEN("it matches the address of the original object") {
        REQUIRE(adjusted_bp != reinterpret_cast<A *>(&b));
      }

      AND_WHEN(
          "the extracted function is called via the adjusted object pointer") {
        b.f(0, 'a', 1.f);
        b.d_ = 1;
        REQUIRE(b.a_ == 1);
        REQUIRE(b.b_ == 'b');
        REQUIRE(b.c_ == 2.f);
        REQUIRE(b.d_ == 1);

        extractedFP(adjusted_bp, 5, 'w', 72.f);

        THEN("the correct effects are observed") {
          REQUIRE(b.a_ == 6);
          REQUIRE(b.b_ == 'x');
          REQUIRE(b.c_ == 144.f);
          REQUIRE(b.d_ == 2);
        }
      }
    }
  }
}
}
}
;
