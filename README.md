<h1 align="center">Welcome to ninjaipc 👋</h1>
<p>
  <a href="https://github.com/oluan/ninjaipc/blob/main/LICENSE" target="_blank">
    <img alt="License: Apache--2.0 License" src="https://img.shields.io/badge/License-Apache--2.0 License-yellow.svg" />
  </a>
</p>

> Easy and simple IPC for Windows and GNU/Linux!



```cpp
ninjahandle ipc = ninjaipc::connect( "ninjaipc" );

ninjaipc::listen( ipc, [ ipc ]( void* request_content ) -> void
{
    std::cout << "Request received!" << std::endl;
    ninjaipc::acknowledge_request( ipc );
});
```

### ✨ [Demo](https://github.com/oluan/ninjaipc/tree/main/example)

## Author

👤 **luan devecchi**

* Github: [@oluan](https://github.com/oluan)

## 🤝 Contributing

Contributions, issues and feature requests are welcome!
Feel free to check [issues page](https://github.com/oluan/ninjaipc/issues). 

## Show your support

Give a ⭐️ if this project helped you!

## 📝 License

Copyright © 2020 [luan devecchi](https://github.com/oluan).
This project is [Apache-2.0 License](https://github.com/oluan/ninjaipc/blob/main/LICENSE) licensed.

***
