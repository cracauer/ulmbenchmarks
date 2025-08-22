void cpp_thrower(int throw_p);

extern int global_blah;
extern int global_blah2;

void test_cpp_testhrow_throw_4(void);
void test_cpp_testhrow_throw_12(void);
void test_cpp_testhrow_throw_24(void);
void test_cpp_testhrow_throw_48(void);

class Foo {
  int printme;
public:
  Foo(int printme_p);
  void testme(void);
  ~Foo();
};
