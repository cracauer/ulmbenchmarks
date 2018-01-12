#! /usr/bin/env python3

import ctiming
import re

def empty(i1, i2):
    return

def plus(p1, p2):
    return p1 + p2

reg1 = re.compile("Canon Image (Width|Height) *: *(\d+)")
def re_pre(p1, p2):
    global reg1
    reg1.match(p1)

def re_inside(p1, p2):
    reg = re.compile("Canon Image (Width|Height) *: *(\d+)")
    reg.match(p1)
    
def re_inline(p1, p2):
    re.search("Canon Image (Width|Height) *: *(\d+)", p1)

di1 = { "foo": 1, "bar": 2, "baz": 3}
di2 = { 1: "foo", 2: "bar", 3: "baz"}

def dictfail(di, key):
    try:
        val = di[key]
    except Exception:
        return False
    return val

def dictnolamba(di, key):
    return di[key]

def exception_notthrown(p1, p2):
    try:
        return p1 + p2 / 42
    except ZeroDivisionError:
        return 0
def exception_thrown(p1, p2):
    try:
        return p1 + p2 / 0
    except ZeroDivisionError:
        return 0

class emptyclass():
    False
    
class simpleclass2():
    def __init__(self):
        self.a = 1
        self.b = 2
        self.c = 3
    def set_a(self, to):
        self.a = to

simpleclass2_instance = simpleclass2()
def setsimpleclass_field_direct(p1, p2):
    simpleclass2_instance.a = p1
def setsimpleclass_field_method(p1, p2):
    simpleclass2_instance.set_a(p1)
        
# fixme, make a library thing out of this
def onerun(description, arg1, arg2, tocall=None):
    if not tocall:
        tocall = eval(description)
    ctiming.onerun(description, arg1, arg2, tocall)

onerun("empty", 0, 0)
onerun("plus", 42, 42)
onerun("empty", 0, 0)

onerun("make empty class instance", 42, 42, lambda p1, p2: emptyclass())
onerun("make simple class instance", 42, 42, lambda p1, p2: simpleclass2())
onerun("setsimpleclass_field_direct", 42, 42)
onerun("setsimpleclass_field_method", 42, 42)

onerun("exception_notthrown", 42, 42)
onerun("exception_thrown", 42, 42)

onerun("small dict str lookup", di1, "bar", lambda di, key: di[key])
onerun("small dict int lookup", di2, 2    , lambda di, key: di[key])
onerun("small dict int lookup failed/ex", di2, 0    , dictfail)
onerun("dictnolamba", di2, 2)


str_for_regex_test = "Canon Image Width asjdfn83yhnfnalksnflknaslfknlaskf78asudmnalsd7asd"
onerun("re_pre", str_for_regex_test, 0)
onerun("re_inside", str_for_regex_test, 0)
onerun("re_inline", str_for_regex_test, 0)
