```bash
cd libmicrohttpd
```

```bash
./configure && make && make install
```

```bash
ls /usr/local/include/microhttpd.h
```

```bash
ls /usr/local/lib/libmicrohttpd.*
```

```bash
clang -o main main.c -I/usr/local/include -L/usr/local/lib -lmicrohttpd
```

```bash
./main 8080
```