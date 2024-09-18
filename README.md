# Sublage 2015

Archive of the complete toolchain for the experimental, stack-based programming langage Sublage

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