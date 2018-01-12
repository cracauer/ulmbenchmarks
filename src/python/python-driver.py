#! /usr/bin/env python

import ctiming
import traceback

# fixme
def onerun(description, arg1, arg2, tocall=None):
    if not tocall:
        tocall = eval(description)
    ctiming.onerun(description, arg1, arg2, tocall)

testmode = False

def do_exception(whoareyou, e, backtrace):
    print("exception from `{}'".format(whoareyou) + ": `{}'".format(e))
    print("`{}'".format(backtrace))
    if testmode:
        raise

avail = []

# some reusable memory
import ctypes
buffer = ctypes.create_string_buffer(1024)

def tmp():
    try:
        import protobuf
    except Exception as e:
        print("Skipping protobufs on import error: '%s'" % (str(e)))
        return
    try:
        print("setting up protobuf tests")
        import simple_pb2
        simple = None
        def pbplus(p1, p2):
            return p1 + p2
        def bufcreate(p1, p2):
            global simple
            simple = simple_pb2.foo()
        def bufsetonefield(p1, p2):
            global simple
            simple.a = p1
        def bufgetonefield(p1, p2):
            global simple
            return simple.b
        def bench_protobufs():
            try:
                print("running protobuf tests")
                onerun("pbplus", 42, 42, pbplus)
                global simple
                simple = simple_pb2.foo()
                onerun("bufcreate", 42, 42, bufcreate)
                onerun("bufsetonefield", 42, 42, bufsetonefield)
                onerun("bufgetonefield", 42, 42, bufgetonefield)
            except Exception as e:
                do_exception("running protobuf tests", e, traceback.format_exc())
        avail.append(bench_protobufs)
    except Exception as e:
        do_exception("defining protobuf tests", e, traceback.format_exc())
tmp()    
        
def tmp():
    try:
        import struct
    except Exception as e:
        print("Skipping struct on import error: '%s'" % (str(e)))
        return
    try:
        print("setting up struct tests")
        data = False
        def stcreate(p1, p2):
            global data
            data = struct.pack("iiii", p1, p2, p1, p2)
        def stinto(p1, p2):
            global buffer
            struct.pack_into("iiii", buffer, 0, p1, p2, p1, p2)
        def stdecode(p1, p2):
            global data
            return struct.unpack("iiii", data)[0]
        def stdecodefrom(p1, p2):
            global buffer
            return struct.unpack_from("iiii", buffer, 0)[0]
        def bench_struct():
            try:
                print("running structs tests")
                onerun("stcreate", 42, 42, stcreate)
                onerun("stinto", 42, 42, stinto)
                onerun("stdecode", 42, 42, stdecode)
                onerun("stdecodefrom", 42, 42, stdecodefrom)
            except Exception as e:
                do_exception("running protobuf tests", e, traceback.format_exc())
        avail.append(bench_struct)
    except Exception as e:
        do_exception("defining struct tests", e, traceback.format_exc())
tmp()    
        
for bench in reversed(avail):
    bench()
