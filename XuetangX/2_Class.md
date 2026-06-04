# 2. Class and Object类与对象
In this chapter, Class and object is discussed. Mainly about what is OOP and why this is efficient for developing.
## 2.1 特性
- 封装
- 继承
- 多态
- 抽象

## 2.2 Date member
### 数据成员声明
- 语法形式
[public | protected | private]
[static][final][transient][volatile]
数据类型 变量名1【=变量初值】，变量名2【=变量初值】，。。。；
- 说明
    - public protected private为访问控制符
    - static指明这是一个静态成员变量（类变量）
    - final指明变量的值不能被修改
    - transient指明变量是不需要序列化的
    - volatile指明变量是一个共享变量。


### 实例变量
- 没有static修饰的变量（数据成员）称为实例变量
- 存储所有实例都需要的属性值可能不同
- 可通过下面的表达式访问实例属性的值 <实例名>.<实例变量名>

### 类变量（静态变量）
- 用static修饰
- 在整个类中只有一个值
- 类初始化的同时就被赋值
- 适用情况
    - 类中所有对象都相同的属性
    - 经常需要共存的数据。
    - 系统中用到的一些常量值。
- 引用格式
<类名|实例名>.<类变量名>
```java
public class Circle {
    static double PI = 3.14159;
    double radius;
}

// test 
public class ClassVariableTester {
    public static void main(String args[]){
        Circle x = new Circle();
        System.out.println(x.PI);
        System.out.println(Circle.PI);
        Circle.PI = 3.14;
        System.out.println(x.PI);
        System.out.println(Circle.PI);
    }
}
// output：
// 3.14159
// 3.14159
// 3.14
// 3.14
```

### 方法成员
类的方法 & 实例的方法

# 2.3 类的重载
**使用this重载构造方法**

Example：
```java
public BankAcount() {
    this("", 999999, 0.0f);
}
public BankAcount(String initName, int initAcountNumber) {
    this(initName, initAcountNumbker, 0.0f);
}
public BankAcount(String initName, int initAcountNumber, float initBalance) {
    ownerName = initName;
    accountNumber = initAcountNuber;
    balance = initBalance;
}
```
final变量的初始化
- 实例变量和类型变量被声明为final
- final实例变量可以在类中定义时给出初始值，或者在每个构造方法结束之前完成初始化；
- final类变量必须在声明的同时初始化


对象自动回收
- 无用对象
    - 离开了作用域的对象
    - 无引用指向的对象
- Java运行时系统通过垃圾收集器周期性地释放无用对象所使用的内存。
- Java运行时系统会在对对象进行自动垃圾回收前，自动调用对象的finalize（）方法。

DicimalFormat类
- DecimalFormat类在java.text包中
- 在toString()方法中使用DecimalFormat类的实例方法format对数据进行格式化。