前端开发入门教程，web前端from 0 to 1
## HTML5

### Schedule

| Date | Course | Comments |
| ------- | ------- | ------- |
| 2 July | P1-P20 | Wow, Treasure founded |
| 3 July | P21-P44 | HTML done |
| 4 July | P45-59 | CSS Basics |

### 灵山就在脚下

### Bilibili Channle: Black house developer
<https://www.bilibili.com/video/BV1Kg411T7t9/?p=2&spm_id_from=pageDriver&vd_source=a61087f39589ca585073e8548bdf0de5>

#### P1 Basics
Course time length: 26h

Basic grammer lectures: 1 - 18h

Project practice: 19 - 26h




#### P2 Pages and its essentials

Code, text, picture, vedeo and motion pictures


#### P3 Browser

Chrome 

Safari 

Internet Explore 


#### P4 Web standard
| Web standard content| element | element's content|
| ------------- |:-------------:| -----:|
|structure:  结构 | HTML | pages's element and content|
|apperance: 表现| CSS   | pages format|
|behavior: 行为| JavaScript |interaction with pages|

#### P5 feel the HTML- a markup language
HTML: Hyper Text Markup Language
超文本标记语言
语法

~~~html
<strong> Guess, Whether this text is bold? </strong>
Here you go.
~~~



#### P6 HTML structure

Just like a passage: a head, body, 

~~~html
<html>
  <head>
    <title> Web page's title </title>
  </head>
  <body>
    The main part of body
  </body>
</html>
~~~

#### P7 VSCode

##### Tools: 
- Visual Studio Code: light and easy to go
- Webstorm: Jetbrain 
- Dreamweaver
- Hbuilder



#### P8 Comments

First **mark** the code is neccesarry at the begining of a new language. So plz learn how to comment you code.

<kbd>Ctrl + / </mkd>

#### P9 Tags
Two tags can appear alone
```html
<br> 
<hr>
```
All others must have two marks

Two Tags' Relations: 
+ father-son
+ borthers




#### P10: Head and Paragraph

Header
H1 
H2
H3
H4
H5
H6

Have and only have 6 tag's of Head

paragragh tag: 
```html
<p>This is a paragraph tag. <br> which hava label 'p' around thes text.</p>
```


#### P11 Image tag

```html
<img src=" " alt = " ">
```




#### P12 Image's basics



#### P13 Image's attribute



#### P14 Image's format



#### P15 Absolute Path



#### P16 comparative path

Category:
> Same directory: <br>
> Son's directory:  images/dog.gif <br>
> Father directory:<br>
> 

upper directory


#### P17 Audio

| Attribute | Function|
| ------- | ------- |
| src | path of the audio file |
| controls | show the button of play |
| autoplay | play automaticly |
| loop | repeated the music play |




```html
<audio src="./music.mp3" controls></audio>
```


#### P18 
This table also suits video DIY.


| Attribute | Function|
| ------- | ------- |
| src | path of the audio file |
| controls | show the button of play |
| autoplay | play automaticly(can play with controller <b>muted</b>) |
| loop | repeated the music play |

#### P19 Video Tag




#### P20 Href

```html
# - empty links: when we wanta to jump, but don't have a target

```
##### Target
|value | effect|
| ------- | ------- |
| _self | Default value(jump at current tab) |
| _blank | jump at a new tab |


#### P21 Case study: Job destrcription of Front developer
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
  <h1>Tencent Front Developer</h1>
</body>
</html>

```


#### P22 Case Study: Links jump-index
Basic structure and two links

#### P23 Case Study: Links jump-others
Can jump to one.index and two.index
one.index: audio contriller
two.index: video controller


#### P24 List tag and Unordered list

Include:
1. Unordered list
2. Ordered list
3. Customized list

#### P25 Ordered list





#### P26 Customized list

Tag ingredient:

|Tag name | Description|
| ------- | ------- |
| dl | 自定义列表的整体，用于包裹dt/dd标签 |
| dt | theme |
| dd | content related to a spersfic theme |





#### P27 Table

Table ingredient:

|Tag name | Description|
| ------- | ------- |
| table | 自定义表格的整体，用于包裹多个tr |
| tr | 表格每行，用于包裹td |
| td | 表格单元格，用于包裹内容 |

嵌套关系：table > tr > td

Table's attribute:

| Attribute Name | Value| Effect|
| ------- | ------- | ------- |
| border | num | border width |
| width | num | unit width |
| height | num | unit height |


#### P28 Table title and title table head

```html
<table border="2">
        <caption><b>Students' Grade 2023 Automn</b></caption>
        <tr>
            <th>Name</th>
            <th>Grade</th>
            <th>Remarks</th>
        </tr>
        <tr>
            <td>Name</td>
            <td>Grade</td>
            <td>Commonts</td>
        </tr>
        <tr>
            <td>John</td>
            <td>100</td>
            <td>Wow, well done!</td>
        </tr>
        <tr>
            <td>Bob</td>
            <td>90</td>
            <td>Cooooooooooooooool!</td>
        </tr>
        <tr>
            <td>Dominic</td>
            <td>150</td>
            <td>This is a wonderful work!</td>
        </tr>
    </table>
```

![Screenshot 2023-07-02 at 14.08.16](inkdrop://file:hQiJwjb6v)


#### P29

|Tag Name | Name |
| ------- | ------- |
| thead | table's header |
| tbody | table's main content |
| tfoot | table's last line |





#### P30 Cancate Tables

|Attribute name| value|Description|
| ------- | ------- | ------- |
| rowspan | num of units | 跨行合并，将多行的单元格垂直合并 |
| colspan | num of units | 跨列合并，将多列的单元格垂直合并 |


#### P31 表单标签
- login
+ regester
* search


#### P32 Input

Input Tag:

|Tag name | type attribute value | Description|
| ------- | ------- | ------- |
| input | text | for text input |
| input | password | for passwort input |
| input | radio | for single selection at multi-options |
| input | checkbox | for multi-selection on multi-options |
| input | file | for file upload |
| input | submit | for submit button |
| input | reset | reset button, for reset |
| input | button | normal button, can work together with JS |





#### P33 PlaceHolder




#### P34 name and checked attribute


#### P35 Upload multiple files



#### P36



#### P37 Button

|Tag name| type attribute value| Description|
| ------- | ------- | ------- |
| button | submit | Submit button, for send data to server after click |
| button | reset | reset button, recover the sheet's original setting |
| button | button | normal button, no operation on default |




#### P38 select下拉菜单

```html
    <select name="" id="">
        <option>Beijing</option>
        <option>Shanghai</option>
        <option>Guangzhou</option>
        <option selected>Shenzhen</option>
    </select>
```





#### P39 Text area

When we input something in a text box:



#### P40 label and radio type input commbination




#### P41 div and span

|Tag | Sytex|
| ------- | ------- |
| header | pages header |
| nav | navigation |
| footer | page footer |
| aside | page aside |
| section | pages blocks |
| article | pages article |

![Screenshot 2023-07-02 at 17.04.24](inkdrop://file:FYST7EjgP)

#### P42 字符实体

![Screenshot 2023-07-02 at 17.07.59](inkdrop://file:A3-THvXZF)


#### P43
Demo code:

```html
 <table border="1">
        <caption><h3>Nice student info table</h3></caption>
        <tr>
            <th>Grade</th>
            <th>Name</th>
            <th>Student Num.</th>
            <th>Class</th>
        </tr>
        <tr>
            <td rowspan="3">Grade3</td>
            <td>John</td>
            <td>110</td>
            <td>Grade3 class2</td>
        </tr>
        <tr>
            <!-- <td>Grade3</td> -->
            <td>Bob</td>
            <td>120</td>
            <td>Grade2 class1</td>
        </tr>
        <tr>
            <!-- <td>Grade3</td> -->
            <td>Dom</td>
            <td>112</td>
            <td>Grade3 class2</td>
        </tr>
        <tr>
            <td>Remarks</td>
            <td colspan="3">You are fatastic!!!!!</td>

        </tr>
    </table>
```


#### P44 Last course about HTML






# CSS3

#### P45 CSS intro

层叠样式表CSS：(Cascading style sheets)

Where?
between title and end of head

```html
<style>
  p {
    color: red;
    font-size: 30px;
  }
  
  
</style>
```
#### P46 Import method

```html
        /* CSS comment */
        /* CSS code here */
        /* Selector {css attribute} */
        /* Selector : search tag you want to edit */
        /* /* p { */
            /* text color */
            /* color: blue; */
            /* font size */
            /* font-size: 30px; */
            /* background-color: green; */
            /* width: 400; */
            /* height: 300; */
        /* } */ */


        /* Now we link the css to the my.css file */
```

##### CSS引入方式：
- 内嵌式： CSS写在style标签中
  - hints: style标签虽然可以写在页面任意位置，但是通常写在head标签中
- 外联式：CSS写在一个单独的.css文件中
  - hints: 需要通过link标签在网页中引入
- 行内式：CSS写在标签上style属性中
  - hints: 基础班不推荐使用，之后会配合js使用

|引入方式 | Position | Scope | Scenario |
| ------- | ------- | ------- |------- |
| nested | CSS written in style tag | current html file | small case |
| linked | CSS written in a new file | muitiple html files | Project |
| in line style | written in style tag | current tag scope | work with JavaScrtipt |




#### P47 tag Selector标签选择器

```html
    <style>
        /* 选择器 {} ： 就是[以标签名命名的选择器]*/
        p {
            color: yellow;
        }
    </style>
```


#### P48 Class Selector类选择器



#### P49 id选择器

```html
<style>
  #two {
    color: red;
  }
</style>  
  <div id="two">This is a test text to test id selector.</div>

```




#### P50 通配符选择器


#### P51 Font and its size

boty default font size: 16

##### 1.2 font-weight

keyword:

| Normal | normal |
| ------- | ------- |
| Bold | bold |

Pure number: 100-900, but Mod(number, 100) = 0

| Normal | 400 |
| ------- | ------- |
| Bold | 700 |

##### 1.3 font-style: italic

font-style value: normal and italic



#### P52 font family

##### 1.4 常见字体系列
- 无衬线字体（sans-serif）
  - 1. 特点：文字笔画粗细均匀，并且首位无装饰
  - 2. 场景：网页中大多采用无衬线字体
  - 3. 常见该系列字体：黑体、Arial
- 衬线字体（serif）
  - 1. 特点：粗细不均匀，有笔锋
  - 2. 场景：报刊书籍中应用广泛
  - 3. 常见该系列字体：宋体、 Time New Roman 
- 等宽字体（monospace）
  - 特点：每个字母或文字的宽度相等
  - 场景：一般用于程序代码编写，有利于代码的阅读和编写
  - 常见该系列字体：Consolas, fira code


![Screenshot 2023-07-04 at 00.29.00](inkdrop://file:wiAJgpUHJ)

##### 1.5 字体和文本样式
windows default; microsoft Yahei
macOS: 苹方



#### P53 层叠

##### 1.6 样式的层叠问题

font字号和字体不能在复合属性书写时省略 


#### P54 font-indent: 2em




#### P55 font-indent



#### P56 水平对齐方式

Attribute: text-align

| value | value |
| ------- | ------- |
| left | left |
| center | center |
| right | right |


##### 2.4 text-align can centers Tag elements


1. text
2. span a
3. input, image

Warning: If you want to center an element, add text-align: center to its <b>father Tag</b> !!!!

#### P57 Text decoration

| Attribute | Effect |
| ------- | ------- |
| underline | draw line under text |
| line-through | rt |
| overline | draw line above text |
| none | no-decoration |


#### P58 Line height

![Screenshot 2023-07-05 at 08.19.56](inkdrop://file:v3RUEP1nB)

行高与font连写的注意点：
- 如果同时设置了行高和font连写，注意覆盖问题
- font: (style wight) size/line-height family;




#### P59 Chrome Debug Tools 

1. See cross-line 
2. Check yeollow !
3. tab key to add new attribute to the end of style 


#### P60 Color value in CSS

| Color show method | Meaning | Attribute value |
| ------- | ------- | ------- |
| Key word | pre-defined color | red, green, blue, yellow |
| rgb | Red-Green-Blue method | rgb(0,0,0) rgb(255,255,255),rgb(255,0,0) |
| rgba | +a transparency | rgba(255,255,255,0.5), rgba(255,0,0,0.3) |
| ox method | #start, turn num into Hex value | #000000, #ff000000, #e92322, or #000, #f00 for short |


#### P61 tag centered 

margin: 0 auto

0: up-down space, when set 0, then left-right will save same space, centered automatically.


#### P62 Comprenhensive case 1: title



#### P63 Comprenhensive case 1: Content



#### P64 Comprenhensive case 2: Picture



#### P65 Comprenhensive case 2: Info and price



#### P66 Selector Advanced

1. 复合选择器
2. 并集选择器
3. 交集选择器
4. Hover伪类选择器
5. Emmet 语法

```html
<style>
  div p {
    color: red;
  }
</style>
```


#### P67 子代选择器





#### P68 并集选择器

```html
<style>
  p, div, span, h1 {
    color: red;
  }
  
</style>

<body
      <p> pppp </p>
      <div> div </div>
      <span> span </span>
      <h1> this is a h1 title tag</h1>
</body>  
```


#### P69 交集选择器

p.box {}

#### P70 hover伪类选择器

```html
<style>
  a:hover {
    color: red;
  }
  
</style>

<body>
      <a href="#">This is an a tag</a>
      <p> pppp </p>
      <div> div </div>
      <span> span </span>
      <h1> this is a h1 title tag</h1>
</body>  
```


#### P71 emmet grammer

VS code can show this **short cut like codes**



#### P72 背景相关属性
#### P73 背景图片 background-image(bgi)


#### P74 背景平铺



#### P75 背景位置




#### P76 背景background



#### P77 背景图和img的区别

#### P78 显示模式-块





#### P79 显示模式-行内

显示特点：
1. 一行可以显示多个
2. 宽度和高度默认由内容撑开
3. 不可以设置宽高

代表标签：
- a
- span
- b
- u
- i
- s
- strong
- ins
- em
- del
- ...

#### P80 行内块元素
 
 显示特点：
 1. 一行可以显示多个
 2. 可以设置宽高

代表标签：
- input, textarea, button, select
- Special Case：img标签有行内块的特点，但是Chrome调试工具中显示结果是inline
 
#### P81 显示模式-转换

目标：能够认识元素显示模式，并通过代码实现元素显示模式的转换

学习路径：
1. 块级元素
2. 行内元素
3. 行内块元素
4. 元素显示模式转换

4.1 元素显示模式转换
目的：改变元素默认的显示特点，让元素符合布局要求

语法：

| Attribute | Effect | Frequency |
| ------- | ------- | ------- |
| display: block | Transfer to block | high |
| display: inline-block | Transfer to inline block | high |
| display: inline | to inline element | low |


#### P82 拓展-标签嵌套

拓展1: HTML嵌套规范注意点

1. 块级元素一般作为大容器，可以嵌套： 文本， 块级元素， 行内元素， 行内块元素……
  - p标签中不嵌套div p h等块元素
2. a标签内部可以嵌套任意元素（除了a标签）

## CSS特性

- 继承性
- 层叠性
- 优先级

#### P83 CSS特性-继承性

1.1 继承性的介绍

子元素有默认继承父元素样式的特点（子承父业）
可以继承的常见属性（** 文字控制属性都可以继承 **）

1. color
2. font-style, font-weight, font-size, font-family
3. text-indent, text-align
4. line-height
5. ……

注意点：可以通过调试工具判断样式是否可以继承



#### P84 CSS特性-层叠性

2.1 层叠性的介绍

- 特性：
  - 给同一个标签设置不同的样式 -〉此时样式会层叠添加 -〉会共同作用在标签上
  - 给同一个标签设置相同的样式 -〉层叠覆盖 -〉最终样式生效

- 样式冲突时，只有当选择器优先级相同时 才能通过层叠性判断结果。

#### P85 综合案例1



#### P86 综合案例2

需要课程图片：五彩导航


#### P87 优先级

3.1 优先级时介绍

- 特性：不同选择器有不同的优先级，优先级高的选择器样式会覆盖优先级低的选择器样式
- 公式：继承 < 通配符选择器 < 标签选择器 < 类选择器 < id选择器 < 行内样式 <!important
- Hints:
  - !important 写在属性值的后面，分号的前面
  - !important 不能提升继承的优先级，只要是继承优先级最低
  - 实际开发中不建议使用 !important




#### P88 优先级-叠加运算





#### P89 优先级-计算题

#### P90 CSS盒子模型-谷歌-排错



#### P91 CSS盒子模型-PxCook




#### P92 CSS盒子模型-组成

##### 1.1 盒子模型的介绍

1. 盒子的概念
   - 页面中的每个标签，都可以看作一个“盒子”， 通过盒子的视角更方便布局
   - 浏览器在渲染（显示）网页时，会将网页中的元素看作是一个个的矩形区域，我们也形象的称为** 盒子 **
3. 盒子模型
    - CSS中规定每个盒子分别由一下元素构成：
      - content
      - padding
      - border
      - margin
 
5. 记忆：可以联想现实中的包装盒


#### P93 CSS盒子模型-07-浏览器效果
#### P94 CSS盒子模型-08-内容width和height





#### P95 CSS盒子模型-09-border的使用方法

- 像素粗细
- 边线种类
- 颜色

| Function | Attribute Name | Value |
| ------- | ------- | ------- |
| Thickness | border-width | Number + px |
| Kind | border-style | solid, dashed and dotted |
| Color | border-color | Color value |


#### P96 CSS盒子模型-10-尺寸计算-border
#### P97 CSS盒子模型-11-案例
#### P98 CSS盒子模型-12-新浪导航-布局div
#### P99 CSS盒子模型-13-内容a
#### P100 CSS盒子模型-14-padding多值
#### P101 CSS盒子模型-15-尺寸-border和padding
#### P102 CSS盒子模型-16-新浪导航-padding优化
#### P103 CSS盒子模型-17-內减模式






#### P104 CSS盒子模型-18-外边距
#### P105 CSS盒子模型-19-清楚默认样式
#### P106 CSS盒子模型-20-版心居中
#### P107 CSS盒子模型-21-新闻列表-div布局
#### P108 CSS盒子模型-22-新闻列表-标题
#### P109 CSS盒子模型-23-新闻列表-内容
#### P110 CSS盒子模型-24-外边距-问题
场景：** 互相嵌套 ** 的 ** 块级元素 ** ，子元素的margin-top会作用在父元素上。

结果：导致父元素一起往下移动

塌陷问题解决方案：
1. 给父元素设置border-top 或者 padding-top（分割父子元素的margin-top）
2. 给父元素设置overflow：hidden （推荐）
3. 转换成行内块元素
4. 设置浮动


#### P111 CSS盒子模型-25-行内元素的内外边距

## CSS 浮动

#### P112-CSS浮动-01-结构伪类-基本用法

目标：能够使用 ** 结构伪类选择器 ** 在HTML中定位元素

1. 使用与优势：
  - Function: Seek element
  - Pros: Reduce the dependency of HTML class
  - Scenario: Seek father selector's child element.
3. 选择器

| Selector | Description |
| ------- | ------- |
| E: first-child() | match the first child as name implies |
| E: last-child() | match the last child as name implies |
| E: n-th child() | the n-th child |
| E: n-th-last-child() | the last n-th child |

#### P113-CSS浮动-02-结构伪类-公式

n can be: 0, 1, 2, 3, ...., 

| Function | Formula |
| :-------: | :-------: |
| Even | 2n, even |
| Odd | 2n + 1, 2n - 1, odd |
| first 5 elements | -n + 5 |
| from the 5-th on | n + 5 |





#### P114-CSS浮动-03-伪元素

#### P115-CSS浮动-04-标准流

#### P116-CSS浮动-05-体验行内块问题

#### P117-CSS浮动-06-作用

#### P118-CSS浮动-07-特点

#### P119-CSS浮动-08-综合案例-小米布局

#### P120-CSS浮动-09-拓展-CSS属性

#### P121-CSS浮动-10-综合案例-小米产品-左右结构

#### P122-CSS浮动-11-综合案例-小米产品-li


#### P123-CSS浮动-12-综合案例-导航


#### P124-CSS浮动-13-清除浮动-场景搭建


#### P125-CSS浮动-14-清除浮动-额外标签


#### P126-CSS浮动-15-清除浮动-单伪元素

2.3 清除浮动的方法-3-单伪元素清除法

Operation: 用伪元素代替了额外标签

1. 基本写法

```html
.clearfix::after {
content: '';
display: block;
clear: both;
}
```

3. 补充写法

```html
 clearfix::after {
content: '';
display: block;
clear: both;
/* added code: cannont not see pseudo-element */
height: 0;
visibility: hidden;
}
```


##### 特点
优点： 项目中使用， 直接给标签加类即可清除浮动




#### P127-CSS浮动-16-清除浮动-双伪元素

Operation:

```html
.clearfix::before,
.clearfix::after {
  content: '';
  display: table;
}
.clearfix::after {
  clear: both;
}
```
特点（优点）：项目中使用，直接给标签加类即可清除浮动



#### P128-CSS浮动-17-清除浮动-overflow

The most simple way:

Add ** overflow: hidden; ** to the father ** .top ** attribute.


#### P129-学成在线项目-01-准备工作

When we develop a website for a company, the code and file should be organized reasonably.

##### 1. Root Directory

Target: Create root dir according to the need of the website.

Root Dir: the first level folder of the website

- Picture folder: images
- Format folder: CSS
- Home page: index.html

```html 
<link rel="stylesheet" href="./css/index.css">
```

#### P130-学成在线项目-02-版心和清除默认样式



#### P131-学成在线项目-03-header布局



#### P132-学成在线项目-04-logo和nav布局



#### P133-学成在线项目-05-nav内容完成



#### P134-学成在线项目-06-搜索-布局和文本框



#### P135-学成在线项目-07-搜索-按钮



#### P136-学成在线项目-08-用户区域


#### P137-学成在线项目-09-banner-布局



#### P138-学成在线项目-10-banner-left完成



#### P139-学成在线项目-11-banner-right-标题



#### P140-学成在线项目-12-banner-righg-内容



#### P141
#### P142
#### P143
#### P144
#### P145
#### P146
#### P147
#### P148
#### P149
#### P150








## Project: Coursera and JD like pages