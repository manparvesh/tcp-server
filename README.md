# tcp-server
TCP server implementation in C

## How to run

- Navigate to the repository using a terminal.
- Create a directory called `build` or something else you like, and navigate into it.
- Run `cmake ..`
- Run `make` to build executables.
- Run the server like this:
```shell
> ./tcp_server_runner 8081 /path/to/dir &
```
- The above command will host all the content present in the specified directory. 
- Now you can access any file with the following extensions using the address inside the specified folder:
  - gif
  - jpg
  - jpeg
  - png
  - ico
  - zip
  - gz
  - tar
  - htm
  - html
- You can also access the contents of the `index.html` file, if present, in the specified folder using the generated executable `tcp_client`.
  - To access a different file using the executable, you can change the `command` variable in `src/client.c`.

# References
- [Implementing a TCP server in C](https://ops.tips/blog/a-tcp-server-in-c/)
- [nweb](https://github.com/ankushagarwal/nweb)