# JavaCoreNote
Notes Repository about Java Core.

## Class

## Inheritance
Keywords: ** extends **, ** autoboxing **;

### java.lang.Integer
- int intValue()
- static String toString(int i)
- static String toString(int i, int radix)
- static int parseInt(String s)
- static int parseInt(String s, int radix)
- static Integer valueOf(String s)
- static Integer valueOf(String s, int radix)

### java.text.NumberFormat
- Number parse(String s)


### java.lang.Enum<E>
- static Enum valueOf(Class enumClass, String name) 
- String toString()
- int ordinal()
- int compareTo(E other)

## Reflection
** Reflection library **, ** reflective **;

### java.lang.Class
- static Class forName(String className)
- Constructor getConstructor(Class... parameterTypes)

### java.lang.reflect.Constructor
- Object newInstance(Object... params)

### java.lang.Throwable
- void printStackTrace() 



## Proxy
proxy/ProxyTest.java ** successful!!! **

## Logging
### LoggingImageeViewer
awt, swing, util and io are imported. 

util.logging.*;

awt.event.*;

### JLabel
~~awt example cannot compile for ** cannot find symbol JLabel in awt."
After check, which only exist in~~
**Shame of typos!!!**
                                                                                                    