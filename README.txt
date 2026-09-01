# Changelog

## Project Description:

h_2 definition is changed.

Sequential h_2 pseudocode -- Lines S4 and S5.

Parallel h_2 pseudocode -- Lines S8 and S15.

Parallel h_1 -- Exclusive read is implemented. The time penalty is insignificant.

Section 5 -- Clearly defined what value to put into the speedup h_1_speedup variable.

## Header file:

get_random_val() function return value is changed from "rand() % k" to "rand() % (n / k)".

struct entry_struct's timestamp field is changed from "clock_t" to "int64_t"

stdint.h header file is included.


# Draft Request

Due to these changes you may need to change your code even if you submitted early. Minimal changes will be required.

1- Header file must be replaced in order to apply these changes mentioned in the previous section.
2- Parallel h_2 and Parallel h_1 code should be changed slightly.
3- Check your "h_1_speedup" value. It's definition in the project description is more clear now. Put the value requested.
4- Make sure your calculated values fit the given variables and fields. There are a few changes happened to the requested types of variables. Check these out.

The draft requests will be recorded and done on the sheet shared below. Edit if you ask.
https://docs.google.com/spreadsheets/d/1mot3-JJa50RTyyyypOJ4pu70iIfB6_3W_wvCHLxtQ_0/edit?usp=sharing


# Testing Your Code

By definition the project is a shared library; therefore, you should not put test functions in the project at all. While implementing it is VERY VERY VERY STRONGLY ADVICED to test your code with your own methods. Testing code or some kind of "hardcoded" values MUST NOT exist in your submitted code. Design your code according to this paradigm.

Optimal way to handle that:
Define a function to create test values.
Save these test values, and put a reading mechanism to recover it later.
Test your code.
While submitting do not include those sections.


# clock_gettime() Function and .timestamp Field

clock() function just paralelly ticks the time and does not measure the actual time. Therefore, the clock() function is replaced with clock_gettime() function. This function fills a previously created "struct timespec" variable. It measures time, and gives the time in "nanoseconds" precision, of course it depends on your hardware, but in our case it is not important, just the time measuring feature. Example code snippet in the project description is changed to project changes you need.

clock_t is essentially long int, it may be 32 or 64 bits, changes by the C implementation you used. It is replaced with int64_t, which is an integer type having a guaranteed size 64 bits. Fix that in your code.

