# 1. Intro about extends of Java Class
git@github.com:flying2322/JavaCoreNote.git
- 继承abstract class：隶属关系 超类->子类

- virtual functions：虚方法


- 泛型：把类型也作为参数

- 组合：部件组装的思想，已有类的对象作为成员变量。

# 3.1 继承
- 已有类定义新类的方法，新类拥有已有类的所有功能
- Java只支持类的单继承，每个子类只能拥有一个直接超类。
-  超类是所有子类的公共属性及方法的集合，子类则是超类的特殊化
- 继承机制可以提高程序的抽象程度，提高代码的可重用性

子类对象
- 外部来看：他应该
    - 与超类相同的接口
    - 可以具有更多的方法和数据成员
- 其内包含着超类所有变量和方法


# 3.2 Object Class


# 3.3 Final Class and Finalize method

# 3.4 abstract class
抽象类和抽象方法

# 3.5 范型
类型作为参数（类，方法）
Example: General class
```java
class GeneralType <Type> {
    Type object;
    public GeneralType(Type object){
        this.object = object;
    }
    public Type getObj(){
        return object;
    }
}

class ShowType {
    public void show (GeneralType<?> o) {
        System.out.printIn(o.getObj().getClass().getName());
    }
}

```

Example: General methods
```java
class GeneralMethod {
    <Type> void printClassName(Type object) {
        System.out.println(object.getClass().getName());
    }
}
public class Test {
    public static void main(Stirng[] args) {
        GenenalMethod  gm = new GeneralMethod();
        gm.printClassName("hello");
        gm.printClassName(3);
        gm.printClassName(3.0f);
        gm.printClassNmae(3.0);
    }
}
```

# 3.6 The combination of Class

 ## 通配符
```java
public class Test {
    public static void main(String args[]) {
        ShowType st = new ShowType();
        GeneralType<Integer> i = new GeneralType<Integer>(2);
        GeneralType<String> s = new GeneralType<String>("hello");
        st.show(i);
        st.show(s);
    }
}
```

Output:
java.lang.Integer
java.lang.String

# General Type with restrictions
"Type" **extends** Interface/super class

```java
class GeneralType<Type extends Number> {
    Type object;
    public GeneralType(Type object) {
        this.object = object;
    }
    public Type getObj() {
        return object;
    }
}
public class Test {
    public static void main(String args[]){
        GeneralType<Integer> i = new GeneralType<Integer> (2);
        // GeneralType<String> s = new GeneralType<String> ("hello");
        // inlegal, T can only be the Class Number or the subclass of Number
    }
}
```

# The combination of Classes
put the exist Class to new Class



# 3.7 Summary

1. Talk abou the re-use mechanism of CLASS, can be [extends] or [combination]s of class
2. The main methods the **Object** have.
3. Final class and finalize methods
4. Abstract class and abstract methods.






















