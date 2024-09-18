# Sublage 2015

Archive of the complete toolchain (compiler, runtime VM, debugger) for the experimental, stack-based programming langage Sublage

```
import console

run <<
    "Hello, World !" println
>>
```


```
import thread
import mutex
import console
import math

var var1

trun <<
    "Hello from thread " over + println
    dup ":" +
    5 do <<
        2 dupn ->string + println
        random 2 % 1 + sleeps
    >> while <<
        1 - dup 0 >
    >> drop2

    "Bye from thread " swap + println
>>

foo <<
    "doo" println
    "bar" println
>>

run <<
    foo
   "Hello var1" ->var1
   "run starting threads" println
   3 do <<
        dup 
        1 
        + 
        ->string 
        @trun 
        thread:start 
        swap
    >> while <<
        1 - dup 0 >
    >> drop
    
   "run waiting for threads end" println
    3 do <<
       swap thread:join
    >> while <<
        1 - dup 0 >
    >> drop
    "run exiting" println
>>
```

```
import console

class inter extends upperclass <<
    
    oncreate <<
        "inter constructor" println
    >>

    f3 <<
        super .f3
        "inter f3" println
     >>
>>

class upperclass <<
    var v1 read write
    
    oncreate <<
        "Upper constructor" println
        "upper ivar v1" self ->.v1
    >>

    f1 <<
        "upper f1" println
    >>

    (f2) <<
        "upper f2" println
    >>

    f3 <<
        "upper f3" println
    >>

    print <<
        "upper v1: " self .v1 + println
    >>
>>


class maclasse extends inter <<

    var v2 write setv2 read getv2 

    var v3 read
    var v4 write 

    oncreate <<
        "Constructor" println
        "ivar v2" self ->.v2
        "ivar v3" self ->.v3
        "ivar v4" self ->.v4
    >>

    print <<
        "v1 : " self .v1 + println
        "v2 : " self .v2 + println
        "v3 : " self .v3 + println
        "v4 : " self .v4 + println
    >>

    ondestroy <<
        "Destructor" println
    >>

    (getv2) <<
        self .v2
    >>

    (setv2) <<
        self ->.v2
    >>

    f1 <<
       super .f1
        "F1" println
        self .f2
    >>

    (f2) <<
        "F2" println
        self .f3
        "toto" self ->.v1
        self .print
    >>

>>
```