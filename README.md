# vc6-asprintf
An asprintf implementation for VC6, C language

This is from [one of my Stack Overflow answers](https://stackoverflow.com/questions/40159892/using-asprintf-on-windows/63317479#63317479).

---

For those with a **higher version of MSVC compiler** (like you're using VS2010) or those using **C++** instead of C, it's easy. You can use the `va_list` implementation in another answer here. It's great.

If you're using a **GCC-based/-like compiler** (like clang, cygwin, MinGW, TDM-GCC etc.), there should be one `asprintf` already, I don't know. If not, you can use the `va_list` implementation in another answer here.

# *An VC6 C Implementation (not C++)*
# Yes, all of the "multi-platform" answers don't support VC6. You have to use this.

(maybe for Turbo C, lcc and any older ones too)

**You can't. You have to:**

1. **Guess a buffer size yourself.**

2. Make a buffer that is **large enough** (which is not easy), then you can get a correct buffer size.

If you choose this, I have make a handy implementation for VC6 C language, based on the `va_list` implement in another answer.

```C
// #include <stdio.h>  /* for _vsnprintf */
// No, you don't need this
#include <stdlib.h> /* for malloc	  */
#include <stdarg.h> /* for va_*	      */
#include <string.h> /* for strcpy     */

// Note: If it is not large enough, there will be fine
// Your program will not crash, just your string will be truncated.
#define LARGE_ENOUGH_BUFFER_SIZE 256

int vasprintf(char **strp, const char *format, va_list ap)
{
	char buffer[LARGE_ENOUGH_BUFFER_SIZE] = { 0 }, *s;
		// If you don't initialize it with { 0 } here,
		// the output will not be null-terminated, if
		// the buffer size is not large enough.

	int len,
		retval = _vsnprintf(buffer, LARGE_ENOUGH_BUFFER_SIZE - 1, format, ap);
		// if we pass LARGE_ENOUGH_BUFFER_SIZE instead of
		// LARGE_ENOUGH_BUFFER_SIZE - 1, the buffer may not be
		// null-terminated when the buffer size if not large enough
	
	if ((len = retval) == -1) // buffer not large enough
		len = LARGE_ENOUGH_BUFFER_SIZE - 1;
		// which is equivalent to strlen(buffer)
			
	s = malloc(len + 1);
	
	if (!s)
		return -1;
	
	strcpy(s, buffer);
		// we don't need to use strncpy here,
		// since buffer is guranteed to be null-terminated
		// by initializing it with { 0 } and pass
		// LARGE_ENOUGH_BUFFER_SIZE - 1 to vsnprintf
		// instead of LARGE_ENOUGH_BUFFER_SIZE
	
	*strp = s;
	return retval;
}

int asprintf(char **strp, const char *format, ...)
{
	va_list ap;
	int retval;
	
	va_start(ap, format);
	retval = vasprintf(strp, format, ap);
	va_end(ap);
	
	return retval;
}

int main(void)
{
	char *s;
	asprintf(&s, "%d", 12345);
	puts(s);

	free(s);
	// note that s is dynamically allocated
	// though modern Windows will free everything for you when you exit
	// you may consider free those spaces no longer in need in real programming
	// or when you're targeting older Windows Versions.

	return 0;
}
```

If you want to know more details, like why have we set a **large enough** buffer size, see below.


## 1. *Explanation*

`snprintf` enters the standard library in C99, and is **not present** in VC6. All you have is a `_snprintf`, which:

1. Returns `-1` if the number of characters to write is less than or equal to `count` (the argument). So can't be used to get the buffer size.

This seems not documentated (see [Microsoft Docs](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/snprintf-snprintf-snprintf-l-snwprintf-snwprintf-l?view=vs-2019)). But `_vsnprintf` has special behavior in the same situation, so I guess there may be something here and with the test below I found my assumption correct.

Yes, it even doesn't return the number of characters it has written, like `_vsnprintf`. Just a `-1`.

2. This one is documentated: `If buffer is a null pointer and count is nonzero, or if format is a null pointer, the invalid parameter handler is invoked, as described in Parameter Validation.` *invalid parameter handler is invoked* means you will get a segmentation fault.

Test code here:

```c
#include <stdio.h>

int main(void)
{
	char s[100], s1[100] = { 0 };

#define TEST(s) printf("%s: %d\n", #s, s)
	
	TEST(_snprintf(NULL, 0, "%d", 12345678));
	/* Tested, and segmentation Fault */
		// TEST(_snprintf(NULL, 100, "%d", 12345678));
	TEST(_snprintf(s, 0, "%d", 12345678));
	TEST(_snprintf(s, 100, "%d", 12345678));
	TEST(_snprintf(s1, 5, "%d", 12345678));
	
	puts(s);
	puts(s1);
	
	return 0;
}
```

And the output with VC6 compiler:

```
_snprintf(NULL, 0, "%d", 12345678): -1
_snprintf(s, 0, "%d", 12345678): -1
_snprintf(s, 100, "%d", 12345678): 8
_snprintf(s1, 5, "%d", 12345678): -1
12345678
12345
```

which supports my assumption.

I initialized `s1` with `{0}`, otherwise it will *not* be null-terminated. `_snprintf` doesn't do that, since the `count` argument is too small.

If you add some `puts`, you will find that second _vsnprintf returns -1 doesn't write anything into `s`, since we passed `0` as the count argument.

Note that when the `count` argument passed in is smaller than the actual string length to write, though `_snprintf` returns -1, it will actually write `count` characters into the buffer.

## 2. *Using vscprintf? No way!*

snprintf enters the standard library in C99, and there is no snprintf, _vsnprintf and __vscprintf:

```
asprintf.obj : error LNK2001: unresolved external symbol _vsnprintf
asprintf.obj : error LNK2001: unresolved external symbol __vscprintf
```

So you can't use the `va_list` implementation in one of the answers.

Actually, there is `_vsnprintf` in VC6, see 3. below. But `_vscprint` is *really* absent.

## 3. *`_vsnprint` & `_snprintf`: Present but Absent*

Actually, `_vsnprintf` is present. If you try to call it, you can make it.

You may say, there is a contradictory, you just said `unresolved external symbol _vsnprintf`. This is weird, but it's true. The `_vsnprintf` in `unresolved external symbol _vsnprintf` is not the one your code links to if you writes `_vsnprintf` directly.

Same thing happens on `_snprintf`. You can call it yourself, but you if you call `snprintf`, the linker will complain that there is no `_snprintf`.

## 4. *Get the buffer size to write by passing 0 argument as count like we do in \*nix? No way!*

What's worse, you can't write this for yourself:

	size_t nbytes = snprintf(NULL, 0, fmt, __VA_ARGS__) + 1; /* +1 for the '\0' */
	char *str = malloc(nbytes);
	snprintf(str, nbytes, fmt, __VA_ARGS__);

That's because:

1. As explained above, there is no `snprintf` in VC6.
2. As explained above, you can replace `snprintf` with `_snprintf` and compile it successfully. But since you passed `NULL`, you will get a segmentation fault.
3. Even if for some reason your program not crashed, `nbytes` will be `-1` since you passed `0`. And `size_t` is usually `unsigned`, so `-1` will become a large number, like 4294967295 in an x86 machine, and your program will stop in the next `malloc` step .

## 5. Maybe a better solution

You can link a library called legacy stdio definitions or something else, but I choose to guess the buffer size myself, since in my case it is not very dangerous to do that.
