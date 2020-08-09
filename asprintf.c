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
	return 0;
}
