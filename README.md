# NinjaIPC: Effortless Inter-Process Communication

NinjaIPC is your go-to library for straightforward, efficient inter-process communication. It's designed to make IPC as simple as possible, without sacrificing performance. With a small footprint and an easy-to-understand blocking API, NinjaIPC enables you to focus on your application, not the plumbing.

## 🌟 Highlights

- **Simplicity First**: Easy to set up and start using immediately.
- **Small & Efficient**: Minimal codebase with a tiny footprint.
- **User-Friendly Blocking API**: We handle the complexities so you can get straight to coding.

## 🌐 Platform Support

| Platform  | Support         |
|-----------|-----------------|
| Windows   | ✅ Supported     |
| Linux     | ✅ Supported     |
| macOS     | 🔜 Coming Soon  |

## 🛠 Language Bindings

| Language  | Status          |
|-----------|-----------------|
| C++       | ✅ Officialy supported |
| Java      | 🚀 Coming Soon  |
| Node.js   | 🚀 Coming Soon  |

## 📚 C++ API Example

The NinjaIPC library also provides a C++ API that makes it even easier to perform inter-process communication.
Below is a simple example demonstrating how to use the `Channel` class, to communicate a simple int32 back and forth.

```cpp
/* Server */
#include <ninjaipc.h>

using namespace NinjaIPC;

int main() {
    auto channel = Channel::make("Readme", sizeof(int));
    while (true) {
        auto i = channel->receive<int>();
        int r = 321;
        channel->reply(r);
        std::cout << "Received " << i << " sent " << r << '\n';
    }
}

/* Client */
#include <ninjaipc.h>

using namespace NinjaIPC;

int main() {
    auto channel = Channel::connect("Readme", sizeof(int));
    while (true) {
        int i = 123;
        int r = channel->send(i);
        std::cout << "Sent " << i << " Received " << r << '\n';
    }
}
```

Running server then client, will produce this output:

```
Received 123 sent 321
Received 123 sent 321
Received 123 sent 321
...

Sent 123 Received 321
Sent 123 Received 321
Sent 123 Received 321
...
```


## 📄 License

The source code is licensed under the [Apache License 2.0](LICENSE).
