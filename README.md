## About

It's a custom preprocessor for the C programming language. It allows you to write generic codes.

## Why you should use it?

In programming, we often need to create data structures or functions that are generic. But in C, it's not supported at all. So instead, we can use this custom preprocessor to add generics to C.

## Working

It generates valid C code from the custom syntax. Its syntax is very similar to C++ but not very flexible. So, you should write your code very carefully to ensure that it doesn't break.

## Usage

### GNU/Linux:

To build the preprocessor, you can use the following commands:

```
$ git clone https://github.com/SohamMalakar/Generics-in-C.git
$ cd Generics-in-C
$ make
```

Now, use this command to run the preprocessor on the provided sample:

```
$ ./generics samples/test.c
```

> **_NOTE:_** Make sure you have git, g++ and make installed in your system.

### Windows:

You don't need to compile the preprocessor on Windows. It's already compiled and ready to use.

Follow the instructions below to run the preprocessor:

1. Download the preprocessor from the [release](https://github.com/SohamMalakar/Generics-in-C/releases/latest) page.
2. Extract the file. (You can use [7-Zip](https://www.7-zip.org/) to extract.)
3. Open the command prompt in the extracted folder.
4. Run the following command:

   ```
   > generics.exe [FILENAME]
   ```

Here, [FILENAME] refers to the name of the file you want to run the preprocessor on.

#### Building the preprocessor for Windows (Not required):

If you want to build the preprocessor, follow the instructions below:

1. [Download](https://github.com/SohamMalakar/Generics-in-C/archive/refs/heads/master.zip) the repository.
2. Extract the archive. (You can use [7-Zip](https://www.7-zip.org/) to extract the archive.)
3. Open the `Generics-in-C` folder.
4. Run the `build.bat` file.

> **_NOTE:_** Make sure you have the latest [g++](https://winlibs.com/) compiler installed in your system.

## Syntax (Similar to C++)

The syntax of the preprocessor is very similar to C++.

```cpp
#include <stdio.h>

template <typename T>
T Add(T a, T b)
{
    return a + b;
}

template <typename T>
T Subtract(T a, T b)
{
    return a - b;
}

template <typename T>
T Multiply(T a, T b)
{
    return a * b;
}

template <typename T>
struct Point
{
    T x;
    T y;
};

int main()
{
    int a = 5;
    int b = 10;

    printf("%d\n", Add<int>(a, b));

    Point<float> p1 = {1, 2};
    Point<float> p2 = {3, 4};

    printf("%f\n", Multiply<float>(p1.x, p2.y));

    return 0;
}
```

It will generate the following code:

```c
#include <stdio.h>

int Add_int(int a, int b)
{
    return a + b;
}

float Multiply_float(float a, float b)
{
    return a * b;
}

struct Point_float
{
    float x;
    float y;
};

int main()
{
    int a = 5;
    int b = 10;

    printf("%d\n", Add_int(a, b));

    struct Point_float p1 = {1, 2};
    struct Point_float p2 = {3, 4};

    printf("%f\n", Multiply_float(p1.x, p2.y));

    return 0;
}
```

So clearly, the generated code is valid C code.

## Samples

There are some examples in the [samples](https://github.com/SohamMalakar/Generics-in-C/tree/master/samples) folder.

## Contribution

If you want to contribute, please feel free to open an [issue](https://github.com/SohamMalakar/Generics-in-C/issues) or [pull request](https://github.com/SohamMalakar/Generics-in-C/pulls).
