// g++ pmf.cpp -std=c++11 -Wno-pmf-conversions -Wall -o pmf && ./pmf
#define CATCH_CONFIG_MAIN
// https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp
#include "catch.hpp"

enum class result_t {
  single,
  base,
  mid1,
  mid2,
  derived,
  existential_crisis
};

SCENARIO("Pointers to member functions are callable and castable",
         "[pmf-call-cast]") {

  GIVEN("A single class with a member function") {
    class A {
    public:
      result_t f() { return result_t::single; }
    };
    using fT = result_t (A::*)();
    WHEN("a pointer to member function is taken") {
      fT memp = &A::f;
      THEN("it is callable on the instance") {
        auto a = A{};
        REQUIRE((a.*memp)() == result_t::single);
      }
    }
  }

  GIVEN("A simple class hierarchy with a non-virtual member function") {
    class B {
    public:
      result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class D : public B {};
    using fTD = result_t (D::*)();

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("it is callable on the base instance") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }

      THEN("it is callable on the derived instance") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::base);
      }

      WHEN("it is cast to a derived member function") {
        fTD memp2 = memp;

        THEN("it is callable on the derived instance") {
          auto d = D{};
          REQUIRE((d.*memp2)() == result_t::base);
        }
      }
    }

    WHEN("a pointer to derived member function is taken") {
      fTD memp = &D::f;

      THEN("it is callable on the derived instance") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::base);
      }
    }
  }

  GIVEN("A diamond class hierarchy with a non-virtual member function") {
    class B {
    public:
      result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class M1 : public B {};
    using fTM1 = result_t (M1::*)();
    class M2 : public B {};
    class D : public M1, public M2 {};

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("it is callable on the base instance") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }
    }

    WHEN("a pointer to a middle-layer member function is taken") {
      fTM1 memp = &M1::f;
      THEN("it is callable on the middle-layer instance") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::base);
      }
      THEN("it is callable on the derived instance") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::base);
      }
    }
  }

  GIVEN("A virtual class hierarchy with a non-virtual member function") {
    class B {
    public:
      result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class M1 : public virtual B {};
    class M2 : public virtual B {};
    class D : public M1, public M2 {};

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("it is callable on the base instance") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }

      THEN("it is callable on the derived instance") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::base);
      }
    }
  }

  GIVEN("A simple class hierarchy with a virtual member function") {
    class B {
    public:
      virtual result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class D : public B {
    public:
      D() : i_(60) {}
      // Ensure we really are an instance of D, not something else.
      result_t f() override {
        return i() == 60 ? result_t::derived : result_t::existential_crisis;
      }
      int i() { return i_; }
      int i_;
    };
    using fTD = result_t (D::*)();

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("calling on the base class returns the base result") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }

      WHEN("it is cast to a derived member function") {
        fTD memp2 = memp;

        THEN("calling on the derived class returns the derived result") {
          auto d = D{};
          REQUIRE((d.*memp2)() == result_t::derived);
        }
      }
    }

    WHEN("a pointer to derived member function is taken") {
      fTD memp = &D::f;

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }
    }

    // mem_fn syntax
    WHEN("the base member function is held in a mem_fn") {
      auto memp = std::mem_fn(&B::f);

      THEN("calling the mem_fn against the base instance returns the base "
           "result") {
        auto b = B{};
        REQUIRE(memp(&b) == result_t::base);
      }

      THEN("calling the mem_fn against the derived instance returns the "
           "derived result") {
        auto d = D{};
        REQUIRE(memp(&d) == result_t::derived);
      }
    }

    WHEN("the derived member function is held in a mem_fn") {
      auto memp = std::mem_fn(&D::f);

      THEN("calling the mem_fn against the derived instance returns the "
           "derived result") {
        auto d = D{};
        REQUIRE(memp(&d) == result_t::derived);
      }
    }

#if (__GNUC__ && __cplusplus) && !__clang__
    // GCC C++ extension: Bound Member Functions (-Wno-pmf-conversions)
    WHEN("the base member function is devirtualised against a base instance") {
      fTB mempB = &B::f;

      // Devirtualise against b
      auto b = B{};
      using fTBnorm = result_t (*)(B *);
      fTBnorm mempBdevirt = reinterpret_cast<fTBnorm>(b.*mempB);

      THEN("calling the devirtualised function pointer against the base "
           "instance returns the base result") {
        REQUIRE(mempBdevirt(&b) == result_t::base);
      }
    }

    WHEN("the base member function is devirtualised against a derived "
         "instance") {
      fTB mempB = &B::f;

      // Devirtualise against d
      auto d = D{};
      using fTBnorm = result_t (*)(B *);
      fTBnorm mempBdevirt = reinterpret_cast<fTBnorm>(d.*mempB);

      THEN("calling the devirtualised function pointer against the derived "
           "instance returns the derived result") {
        REQUIRE(mempBdevirt(&d) == result_t::derived);
      }

      THEN("calling the static derived function pointer against a base "
           "instance demonstrates that we've subverted the type system") {
        auto b = B{};
        fTBnorm mempDnorm = &D::f;
        REQUIRE(mempDnorm(&b) == result_t::existential_crisis);
        // Just in case it isn't clear, the following call is what we did
        // in the above call.
        REQUIRE(static_cast<D *>(&b)->D::f() == result_t::existential_crisis);
      }

      THEN("calling the devirtualised function pointer against a base "
           "instance demonstrates that we've subverted the type system") {
        auto b = B{};
        REQUIRE(mempBdevirt(&b) == result_t::existential_crisis);
        // Just in case it isn't clear, the following call is what we did
        // in the above call.
        REQUIRE(static_cast<D *>(&b)->D::f() == result_t::existential_crisis);
      }
    }
#endif // (__GNUC__ && __cplusplus) && !__clang__
  }

  GIVEN("A diamond class hierarchy with a virtual member function") {
    class B {
    public:
      virtual result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class M1 : public B {
    public:
      virtual result_t f() override { return result_t::mid1; }
    };
    using fTM1 = result_t (M1::*)();

    class M2 : public B {
    public:
      virtual result_t f() override { return result_t::mid2; }
    };

    class D : public M1, public M2 {
    public:
      virtual result_t f() override { return result_t::derived; }
    };
    using fTD = result_t (D::*)();

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("calling on the base class returns the base result") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }

      THEN(
          "calling on the middle-layer class returns the middle-layer result") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::mid1);
      }
    }

    WHEN("a pointer to a middle-layer member function is taken") {
      fTM1 memp = &M1::f;

      THEN(
          "calling on the middle-layer class returns the middle-layer result") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::mid1);
      }

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }
    }

    WHEN("a pointer to derived member function is taken") {
      fTD memp = &D::f;

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }
    }
  }

  GIVEN("A virtual class hierarchy with a virtual member function") {
    class B {
    public:
      result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class M1 : public virtual B {
    public:
      virtual result_t f() { return result_t::mid1; }
    };
    using fTM1 = result_t (M1::*)();

    class M2 : public virtual B {
    public:
      virtual result_t f() { return result_t::mid2; }
    };

    class D : public M1, public M2 {
    public:
      virtual result_t f() { return result_t::derived; }
    };
    using fTD = result_t (D::*)();

    WHEN("a pointer to base member function is taken") {
      fTB memp = &B::f;

      THEN("calling on the base class returns the base result") {
        auto b = B{};
        REQUIRE((b.*memp)() == result_t::base);
      }

      THEN(
          "calling on the middle-layer class returns the middle-layer result") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::base);
      }

      THEN(
          "calling on the middle-layer class returns the middle-layer result") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::base);
      }
    }

    WHEN("a pointer to a middle-layer member function is taken") {
      fTM1 memp = &M1::f;

      THEN(
          "calling on the middle-layer class returns the middle-layer result") {
        auto m1 = M1{};
        REQUIRE((m1.*memp)() == result_t::mid1);
      }

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }
    }

    WHEN("a pointer to derived member function is taken") {
      fTD memp = &D::f;

      THEN("calling on the derived class returns the derived result") {
        auto d = D{};
        REQUIRE((d.*memp)() == result_t::derived);
      }
    }
  }
}

SCENARIO("Pointers to member functions are comparable", "[pmf-comparison]") {
  GIVEN(
      "A simple class hierarchy with a virtual member function and override") {
    class B {
    public:
      virtual result_t f() { return result_t::base; }
    };
    using fTB = result_t (B::*)();

    class D : public B {
    public:
      result_t f() override { return result_t::derived; }
    };

#if (__GNUC__ && __cplusplus) && !__clang__
    // GCC C++ extension: Bound Member Functions (-Wno-pmf-conversions)
    WHEN("devirtualised pointers to the member functions are taken") {
      using fTBnorm = result_t (*)(B *);

      fTBnorm mempBnorm = &B::f;
      fTBnorm mempDnorm = &D::f;

      THEN("they are different") { REQUIRE(mempBnorm != mempDnorm); }

      WHEN(
          "the base member function is devirtualised against a base instance") {
        fTB mempB = &B::f;
        auto b = B{};

        // Devirtualise against b
        fTBnorm mempBdevirt = reinterpret_cast<fTBnorm>(b.*mempB);

        THEN("the devirtualised function pointer is equal to the base function "
             "pointer") {
          REQUIRE(mempBdevirt == mempBnorm);
        }
      }

      WHEN("the base member function is devirtualised against a derived "
           "instance") {
        fTB mempB = &B::f;
        auto d = D{};

        // Devirtualise against d
        fTBnorm mempBdevirt = reinterpret_cast<fTBnorm>(d.*mempB);

        THEN("the devirtualised function pointer is equal to the derived "
             "function pointer") {
          REQUIRE(mempBdevirt == mempDnorm);
        }
      }
    }
#endif // (__GNUC__ && __cplusplus) && !__clang__
  }
}
