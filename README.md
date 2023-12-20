Download libmicrohttpd and install

[Download Tar](https://www.gnu.org/software/libmicrohttpd/#download)
[Copy dist folder from SwaggerUI](https://github.com/swagger-api/swagger-ui/releases/latest)

1. Build project
```bash
./build.sh
```

2. Run project (provide the PORT)
```bash
./build/main 8080
```

3. Host Swagger.JSON file
4. Change swagger-initializer.js file to use the swagger json url
